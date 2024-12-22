
//------------------------------------------------------------------------------
// Parser.cpp
//------------------------------------------------------------------------------


#include "Parser.hpp"

#include <dlfcn.h>

namespace {
    void trim_left(std::string_view& view) {
        while (std::isspace(view[0])) {
            view.remove_prefix(1);
        }
    }

    void trim_right(std::string_view& view) {
        while (std::isspace(view[view.size() - 1])) {
            view.remove_suffix(1);
        }
    }

    void trim_sides(std::string_view& view) {
        trim_left(view);
        trim_right(view);
    }

    bool haswhitespace(const std::string_view view) {
        return std::any_of(view.begin(), view.end(), [] (auto c) { return std::isspace(c); });
    }
}

void ParsedBlock::parse_block(std::string_view input) {
    std::array<char, 3> steps = { '(', '=', ' ' };

    while (!input.empty()) {
        auto step = input.find_first_of("(=;");
        if (step == std::string_view::npos) {
            break;
        } else if (input[step] == '(') {
            //function call
            auto step2 = input.find_first_of(";");
            std::string_view statement(input.begin(), input.begin() + step2 + 1);
            auto call = parse_function_call(statement);
            statements.push_back(std::move(call));
            input.remove_prefix(step2 + 1);
        } else if (input[step] == '=') {
            //assignment
            auto step2 = input.find_first_of(";");
            std::string_view statement(input.begin(), input.begin() + step2 + 1);
            auto assignment = parse_variable_assignment(statement);
            statements.push_back(std::move(assignment));
            input.remove_prefix(step2 + 1);
        } else if (input[step] == ';') {
            //variable definition
            std::string_view statement(input.begin(), input.begin() + step + 1);
            auto def = parse_variable_definition(statement);
            vars.push_back(def);
            input.remove_prefix(step + 1);
        } else {
            throw std::runtime_error("unknown section");
        }
    }
}

FunctionCallPtr ParsedBlock::parse_function_call(std::string_view input) {
    auto call = std::make_unique<FunctionCall>();

    auto nameEnd = input.find_first_of("(");
    if (nameEnd == std::string_view::npos) {
        throw std::runtime_error("Not a function call.");
    }

    auto name = input.substr(0, nameEnd);
    trim_sides(name);
    call->functionName = name;
    input.remove_prefix(nameEnd + 1);

    void* dlHandle = dlopen(0, RTLD_NOW);
    void* function = dlsym(dlHandle, call->functionName.c_str());
    if (!function) {
        throw std::runtime_error("Unknown function name.");
    }
    call->functionAddr = function;

    auto tokenEnd = input.find_first_of(",)");
    while (tokenEnd != std::string_view::npos) {
        auto token = input.substr(0, tokenEnd);
        trim_left(token);

        if (std::isdigit(token[0])) {
            //integer
            std::string valueChars(token);
            auto param = std::make_unique<Int64Param>();
            param->content = std::atoi(valueChars.c_str());
            call->params.push_back(std::move(param));
        } else if (token[0] == '"') {
            //string start
            token.remove_prefix(1);
            auto stringEnd = token.find_first_of('"');
            auto param = std::make_unique<StringParam>();
            param->content = std::string(token.substr(0, stringEnd));
            call->params.push_back(std::move(param));
        } else {
            trim_right(token);
            if (haswhitespace(token)) {
                throw std::runtime_error("unexpected whitesapce");
            }

            auto param = std::make_unique<StackVariableParam>();
            param->content = token;
            call->params.push_back(std::move(param));
        }

        input.remove_prefix(tokenEnd + 1);
        tokenEnd = input.find_first_of(",)");
    }

    if (input[0] != ';') {
        throw std::runtime_error("Unexpected character.");
    }

    return call;
}

VariableDefinition ParsedBlock::parse_variable_definition(std::string_view input) {
    VariableDefinition def;

    trim_left(input);
    auto splitter = input.find_first_of(' ');
    auto type = input.substr(0, splitter);
    trim_sides(type);

    if (type != "int64") {
        throw std::runtime_error("unexpected type");
    }
    def.type = VariableDefinition::Int64;
    input.remove_prefix(splitter + 1);

    splitter = input.find_first_of(';');
    auto name = input.substr(0, splitter);
    trim_sides(name);

    if (haswhitespace(name)) {
        throw std::runtime_error("Unexpected whitespace.");
    }
    def.name = name;

    return def;
}

VariableAssignmentPtr ParsedBlock::parse_variable_assignment(std::string_view input) {
    auto assign = std::make_unique<VariableAssignment>();

    auto splitter = input.find_first_of('=');
    auto assignTo = input.substr(0, splitter);
    trim_sides(assignTo);
    if (haswhitespace(assignTo)) {
        throw std::runtime_error("Unexpected whitespace.");
    }
    assign->to.content = assignTo;

    input.remove_prefix(splitter + 1);

    auto end = input.find_first_of(';');
    auto constant = input.substr(0, end);
    trim_sides(constant);

    auto value = std::make_unique<Int64Param>();
    value->content = atoi(std::string(constant).c_str());
    assign->value = std::move(value);

    return assign;
}