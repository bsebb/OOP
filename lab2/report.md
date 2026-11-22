# Lab 2 report, track 2 (local agent)

Bahna Sebastian  
Repository: https://github.com/bsebb/OOP (branch `lab-comp-aggr-track-2`)

To keep my first assignment intact as a single file, I left `lab1/main.cpp` untouched and placed each class for the new project into its own `.h` and `.cpp` pair under `lab2/src`. From the `lab2/` directory, the command `g++ -std=c++17 -Wall -o lab2 src/*.cpp` builds the executable in one pass. Because macOS links `g++` to Apple clang rather than GNU GCC, I could not verify the binary on a native Linux compiler. Once launched, the program requests a memory tier (`1`, `2`, or `3`) and reads terminal input until `quit`, while `/test` triggers the old validator checks and `/level error` (or `trace`, `info`, `warn`) switches verbosity live. Live commands prevented constant recompilation.

## 1. Splitting the logger in three

Because Lab 1 placed three separate jobs inside `Logger`, namely storing the format string, replacing tokens, and printing lines to `std::cout`, Part A divides that old design across three smaller classes. That old class is now split.

While `Formatter` holds the static `format` string and builds the finished line without printing, `ConsoleSink` wraps `std::cout << line << '\n'` inside one `write(line)` method. Naming the formatting method `apply(level, name, message)` avoided a collision with the static member `format`. Between them sits `Logger`, which stores its `name`, checks the static `level`, fills any `%` slots from the bonus task, queries `formatter->apply`, and hands the result to `sink->write`. Because `Logger::setFormat` no longer exists, callers modify the layout by updating `Formatter::format` or passing a new template to the `Formatter` constructor, and the `%r` random phrase token from Lab 1 extra credit remains supported. Everything from Lab 1 still passes.

Token sequence controls `%n` expansion. When a message itself contains `%n`, the outcome depends of the order of `replaceAll` calls inside `Formatter::apply`, which in Lab 1 ran `%t`, `%l`, `%n`, `%m`, `%r` and let `%r` overwrite message text.

By moving `%m` to the final step (`%t`, `%l`, `%n`, `%r`, then `%m`), `Formatter::apply` never scans the message payload after insertion. Consequently, `demo.info("name in message: %n")` outputs `name in message: %n` without replacing the inner tag. Before `formatter->apply` runs, however, the bonus placeholder loop in `Logger::log` scans the raw message for `%` slots, so an empty argument list leaves `%n` intact while `info("%n", {"x"})` produces `xn`. I left that rule untouched, it matches the bonus description. No other tokens are affected.

## 2. Composition and aggregation

Two constructors define pointer ownership. When a caller invokes the one-argument constructor `Logger(name)`, the object allocates `new Formatter(Formatter::format)` and `new ConsoleSink()` on heap and sets `ownsParts = true` so `~Logger()` deletes both pointers at destruction (composition).

If a caller instead uses the three-argument overload `Logger(name, f, s)`, the instance stores the two external addresses and sets `ownsParts = false`, leaving their cleanup to the outside owner (aggregation). In `main`, one `Formatter` and one `ConsoleSink` reside on the stack and pass their addresses into `Agent` for its three-argument logger, whereas `Memory`, `Toolbox`, `LMStudioClient`, and the default `ParenthesesValidator` call the one-argument version. Passing a `Logger` by value into Lab 1's second `ParenthesesValidator(Logger customLogger)` constructor broke this design on the first run because shallow copying duplicated both pointers with `ownsParts = true`. As soon as the temporary parameter freed those addresses, the validator crashed with exit code 133 (`SIGTRAP` on double free), which I resolved by changing the constructor signature to `ParenthesesValidator(Formatter* f, ConsoleSink* s)`. That one-line change stopped the crash.

Both patterns appear across the agent. While `Agent` borrows `Memory*`, `LMStudioClient*`, and `Toolbox*` from `main`'s stack without deleting them (aggregation), `Memory`, `LMStudioClient`, `Toolbox`, and `Agent` each embed their own `Logger logger` member by value (composition).

## 3. Static vs shared objects, and why level is still static

State is shared through two mechanisms. Because `Logger::level` is a static variable with a single copy in memory, calling `Logger::setLevel(LogLevel::ERROR)` or typing `/level error` silences `TRACE` and `INFO` lines across `Memory`, `Pip`, `LMStudio`, and `Toolbox` simultaneously. Under the specification, `Formatter::format` is also static, meaning every `Formatter` object reads from one global template string. Although passing `&fmt` and `&sink` from `main` into `Agent` is meant to limit sharing to objects holding those addresses, the `Formatter(fmt)` constructor overwrites the static `Formatter::format` string for every formatter. In `runTests()`, constructing `Formatter shared("%l | %n | %m")` for a second validator also mutated the first validator's output.

Static severity still makes sense. If `format` were a normal member variable, passing `&shared` to the aggregation constructor would let a chosen group of loggers share one layout while `level` stayed static as a single switch for the program.

## 4. The tool protocol

Every turn begins with `Agent::systemMessage()`. Inside that method, the prompt defines the name `Pip` and its short-answer personality, lists the four tools, and instructs the model to output only JSON like `{"tool": "calculator", "input": "12*7"}` when invoking a tool. Rather than storing this prompt inside `Memory`, `Agent::chat` prepends it to a local `std::vector<Message>` right before calling `client->send(msgs)`. Because the system prompt stays out of `Memory`, capacity drops in smaller tiers cannot evict the tool rules. Five stages handle each user question:

1. `Agent::chat` pushes the user's line into `Memory`, builds the outgoing vector (`system` message at index 0 followed by `memory->msgs`), logs the message count at `TRACE`, and calls `LMStudioClient::send`.
2. `LMStudioClient::send` escapes special characters into a hand-built JSON string, opens `curl -s -X POST http://localhost:1234/v1/chat/completions ...` with `popen`, reads stdout via `fgets`, and times the call using `std::chrono::steady_clock`. Next, it pulls `choices[0].message.content`, `prompt_tokens`, and `completion_tokens` out with `find` and `substr`, after which `Agent` logs an `INFO` line showing elapsed seconds, token counts, and speed.
3. `Agent::findTool` slices the text from the first `{` to the last `}` and calls `jsonValue` for `"tool"` and `"input"`. If braces or `"tool"` are absent, the text is returned as a normal answer.
4. When `findTool` matches a tool and `toolCalls < 5`, `chat()` records the JSON reply in `Memory` as an `assistant` turn and executes `toolbox->run(tool, input)`. It then appends `Tool <name> returned: <result>` (or an error notice if empty) as a `user` message and repeats from stage 1.
5. Should the model request a sixth tool call in one question (`toolCalls == 5`), `chat()` logs a `WARN`, appends `No more tools. Answer with what you have.` as a `user` turn, and sends one last request.

Prompt tuning fixed early tool skips. Because `qwen3-vl-8b-instruct` initially made the calculation of `12*7` itself and replied `84` without `calculator`, I added `Always use calculator for any arithmetic, even easy sums, never work it out yourself.` to the prompt.

Error handling required two further checks. When I briefly listed `get_weather` in the prompt and asked for the forecast, the model called `get_weather`, `Toolbox::run` logged `ERROR Unknown tool: get_weather` and returned `""`, and `Agent` fed the error back. After receiving `Tool get_weather returned: error, that tool failed or does not exist`, the model replied that it could not check weather data. Stopping LM Studio mid-chat originally threw `std::out_of_range` when `curl` returned an empty string and `out.substr(out.find("\"message\""))` passed `npos` to `substr`. Guarding `LMStudioClient::send` against empty output and missing keys inside a `try/catch` block turned that crash into an `ERROR` log.

Three memory tiers were benchmarked. Using the same 14-question script against `qwen3-vl-8b-instruct` on my laptop produced 20 HTTP requests per tier due to tool calls, with `tokens/s` dividing `completion_tokens` by the full `curl` wall time (including prompt evaluation).

| tier | capacity | drops | avg prompt tokens | max prompt tokens | completion tokens | total time | tokens/s |
|---|---|---|---|---|---|---|---|
| 1 | 6 | 34 | 252 | 291 | 316 | 24.2 s | 13.0 |
| 2 | 20 | 20 | 405 | 518 | 283 | 31.5 s | 9.0 |
| 3 | 60 | 0 | 480 | 772 | 286 | 16.5 s | 17.3 |

Smaller tiers forgot my name. Both tier 1 and tier 2 evicted my opening line (`"my name is Seb"`) before the final question, whereas tier 3 retained all 40 messages and answered with my name. Surprisingly, tier 3 also finished in 16.5 s compared to 31.5 s for tier 2 despite sending the largest prompts. Because tier 3 only appends messages at the end, LM Studio can reuse its KV prefix cache across requests, while every `msgs.erase(msgs.begin())` in tiers 1 and 2 shifts the history after the system prompt and forces full re-evaluation. Each tier ran once without server cache metrics.

## 5. Reflection

Raw pointers caused the hardest bugs. Splitting `Logger` into three files was simple, yet storing heap pointers without a copy constructor triggered a double-free crash as soon as the old validator constructor copied a logger by value.

Hand-written JSON parsing leaves clear limits. Because `Agent::jsonValue` stops at the next quote without checking for `\"` escapes and `LMStudioClient::readString` strips the backslash from `\uXXXX` instead of decoding hex Unicode, complex strings can lose characters. In addition, an apostrophe in `"what's"` broke the single-quoted `curl -d '...'` shell command until `LMStudioClient::escape` replaced every `'` with `'\''`. When LM Studio is offline, `Agent::chat` inserts the user's question into `Memory` before calling `client->send`, leaving an unanswered `user` turn so the next question sends two `user` messages in a row. Delaying that insert until `send` succeeds, adding a deep copy constructor to `Logger`, and separating prompt latency from generation speed would clean up the design.
