#include <sstream>
#include "Agent.h"

Agent::Agent(const std::string& n, const std::string& p, Memory* m, LMStudioClient* c, Toolbox* t,
             Formatter* f, ConsoleSink* s)
    : name(n), personality(p), memory(m), client(c), toolbox(t), logger(n, f, s) {
    toolCalls = 0;
}

std::string Agent::systemMessage() {
    std::string sys = "You are " + name + ", " + personality + ".\n";
    sys += "You can use tools. To use one, reply with only a JSON object like "
           "{\"tool\": \"calculator\", \"input\": \"12*7\"} and nothing else.\n";
    sys += "Tools:\n";
    sys += "get_time: the current time, no input\n";
    sys += "get_date: today's date, no input\n";
    sys += "calculator: two numbers and one operator (+ - * /), for example 12*7\n";
    sys += "roll_dice: number of sides, for example 6\n";
    sys += "Always use calculator for any arithmetic, even easy sums, never work it out yourself.\n";
    sys += "When you get a message like \"Tool calculator returned: 84\", use it to answer the user "
           "in plain text. Only use a tool when you need one.";
    return sys;
}

std::string Agent::jsonValue(const std::string& json, const std::string& key, bool& found) {
    found = false;
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) {
        return "";
    }
    pos = json.find(':', pos + key.size() + 2);
    if (pos == std::string::npos) {
        return "";
    }
    pos++;
    while (pos < json.size() && json[pos] == ' ') {
        pos++;
    }
    if (pos >= json.size()) {
        return "";
    }

    std::string value = "";
    if (json[pos] == '"') {
        size_t end = json.find('"', pos + 1);
        if (end == std::string::npos) {
            return "";
        }
        value = json.substr(pos + 1, end - pos - 1);
    } else {
        // numbers like "input": 20 have no quotes
        size_t end = json.find_first_of(",}", pos);
        value = json.substr(pos, end - pos);
        while (!value.empty() && value[value.size() - 1] == ' ') {
            value = value.substr(0, value.size() - 1);
        }
    }
    found = true;
    return value;
}

bool Agent::findTool(const std::string& text, std::string& tool, std::string& input) {
    // the json is sometimes wrapped in text or ``` so take first { to last }
    size_t start = text.find('{');
    size_t end = text.rfind('}');
    if (start == std::string::npos || end == std::string::npos || end < start) {
        return false;
    }

    std::string json = text.substr(start, end - start + 1);
    bool found = false;
    tool = jsonValue(json, "tool", found);
    if (!found || tool.empty()) {
        return false;
    }
    input = jsonValue(json, "input", found);
    return true;
}

std::string Agent::chat(const std::string& question) {
    memory->add(Message("user", question));
    toolCalls = 0;

    std::string answer = "";
    bool done = false;
    while (!done) {
        // system message goes in front every time but never into memory
        std::vector<Message> msgs;
        msgs.push_back(Message("system", systemMessage()));
        for (int i = 0; i < (int)memory->msgs.size(); i++) {
            msgs.push_back(memory->msgs[i]);
        }
        logger.trace("Request sent with % messages", {std::to_string(msgs.size())});

        LMResponse r = client->send(msgs);
        if (r.text.empty()) {
            logger.error("Could not read an answer, giving up on this question");
            return "";
        }

        double tps = 0;
        if (r.seconds > 0) {
            tps = r.completionTokens / r.seconds;
        }
        std::ostringstream speed;
        speed.precision(3);
        speed << tps;
        logger.info("Answer in %s: % prompt tokens, % completion tokens, % tokens/s",
                    {std::to_string(r.seconds).substr(0, 4), std::to_string(r.promptTokens),
                     std::to_string(r.completionTokens), speed.str()});

        std::string tool = "";
        std::string input = "";
        if (findTool(r.text, tool, input)) {
            if (toolCalls < 5) {
                toolCalls++;
                memory->add(Message("assistant", r.text));
                std::string result = toolbox->run(tool, input);
                logger.trace("Tool % gave '%'", {tool, result});
                if (result.empty()) {
                    result = "error, that tool failed or does not exist";
                }
                memory->add(Message("user", "Tool " + tool + " returned: " + result));
                continue;
            } else if (toolCalls == 5) {
                toolCalls++;
                logger.warn("Tool limit reached, asking for a plain answer");
                memory->add(Message("assistant", r.text));
                memory->add(Message("user", "No more tools. Answer with what you have."));
                continue;
            }
        }

        answer = r.text;
        done = true;
    }

    memory->add(Message("assistant", answer));
    return answer;
}
