#include <chrono>
#include <cstdio>
#include "LMStudioClient.h"

const std::string MODEL = "qwen3-vl-8b-instruct-abliterated";

LMStudioClient::LMStudioClient() : logger("LMStudio") {}

std::string LMStudioClient::escape(const std::string& s) {
    std::string out = "";
    for (int i = 0; i < (int)s.size(); i++) {
        char c = s[i];
        if (c == '"') {
            out += "\\\"";
        } else if (c == '\\') {
            out += "\\\\";
        } else if (c == '\n') {
            out += "\\n";
        } else if (c == '\r') {
            out += "\\r";
        } else if (c == '\t') {
            out += "\\t";
        } else if (c == '\'') {
            // the body sits inside '...' in the shell, so close it, add a quote, reopen
            out += "'\\''";
        } else {
            out += c;
        }
    }
    return out;
}

std::string LMStudioClient::buildBody(const std::vector<Message>& msgs) {
    std::string body = "{\"model\": \"" + MODEL + "\", \"temperature\": 0.7, \"messages\": [";
    for (int i = 0; i < (int)msgs.size(); i++) {
        if (i > 0) {
            body += ", ";
        }
        body += "{\"role\": \"" + msgs[i].role + "\", \"content\": \"" + escape(msgs[i].content) + "\"}";
    }
    body += "]}";
    return body;
}

std::string LMStudioClient::readString(const std::string& json, size_t pos) {
    std::string text = "";
    pos = json.find('"', pos) + 1; // skip the opening quote
    while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\' && pos + 1 < json.size()) {
            pos++;
            char e = json[pos];
            if (e == 'n') {
                text += '\n';
            } else if (e == 't') {
                text += '\t';
            } else if (e == 'r') {
                // skip
            } else {
                text += e;
            }
        } else {
            text += json[pos];
        }
        pos++;
    }
    return text;
}

int LMStudioClient::readNumber(const std::string& json, const std::string& key) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) {
        return 0;
    }
    pos = json.find(':', pos) + 1;
    size_t end = json.find_first_of(",}", pos);
    return std::stoi(json.substr(pos, end - pos));
}

LMResponse LMStudioClient::send(const std::vector<Message>& msgs) {
    LMResponse r;
    r.text = "";
    r.promptTokens = 0;
    r.completionTokens = 0;
    r.seconds = 0;

    std::string cmd = "curl -s -X POST http://localhost:1234/v1/chat/completions "
                      "-H \"Content-Type: application/json\" -d '" + buildBody(msgs) + "'";

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    FILE* pipe = popen(cmd.c_str(), "r");
    std::string out = "";
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        out += buffer;
    }
    pclose(pipe);

    std::chrono::steady_clock::time_point stop = std::chrono::steady_clock::now();
    r.seconds = std::chrono::duration<double>(stop - start).count();

    size_t pos = out.find("\"message\"");
    std::string rest = out.substr(pos);
    pos = rest.find("\"content\":");
    r.text = readString(rest, pos + 10);
    r.promptTokens = readNumber(out, "prompt_tokens");
    r.completionTokens = readNumber(out, "completion_tokens");
    return r;
}
