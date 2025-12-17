#include "parser.h"
#include "interpreter.h"
#include "functions.h"
#include <iostream>
#include <memory>
#include <algorithm>

// Expression classes
class LiteralExpr : public Expression {
public:
    explicit LiteralExpr(const Value& val) : value_(val) {}
    Value evaluate(Interpreter* interp) override { return value_; }
private:
    Value value_;
};

class VariableExpr : public Expression {
public:
    explicit VariableExpr(const std::string& name) : name_(name) {}
    Value evaluate(Interpreter* interp) override {
        return interp->getVariables().getVariable(name_);
    }
private:
    std::string name_;
};

class BinaryExpr : public Expression {
public:
    BinaryExpr(std::shared_ptr<Expression> left, TokenType op, std::shared_ptr<Expression> right)
        : left_(left), op_(op), right_(right) {}
    
    Value evaluate(Interpreter* interp) override {
        Value lval = left_->evaluate(interp);
        Value rval = right_->evaluate(interp);
        
        switch (op_) {
            case TokenType::PLUS: return lval + rval;
            case TokenType::MINUS: return lval - rval;
            case TokenType::MULTIPLY: return lval * rval;
            case TokenType::DIVIDE: return lval / rval;
            case TokenType::EQUAL: return Value(lval == rval ? 1.0 : 0.0);
            case TokenType::NOT_EQUAL: return Value(lval != rval ? 1.0 : 0.0);
            case TokenType::LESS: return Value(lval < rval ? 1.0 : 0.0);
            case TokenType::GREATER: return Value(lval > rval ? 1.0 : 0.0);
            case TokenType::LESS_EQUAL: return Value(lval <= rval ? 1.0 : 0.0);
            case TokenType::GREATER_EQUAL: return Value(lval >= rval ? 1.0 : 0.0);
            case TokenType::AND: 
                return Value((lval.getNumber() != 0 && rval.getNumber() != 0) ? 1.0 : 0.0);
            case TokenType::OR:
                return Value((lval.getNumber() != 0 || rval.getNumber() != 0) ? 1.0 : 0.0);
            default: return Value(0.0);
        }
    }
private:
    std::shared_ptr<Expression> left_;
    TokenType op_;
    std::shared_ptr<Expression> right_;
};

class FunctionCallExpr : public Expression {
public:
    FunctionCallExpr(TokenType func, std::vector<std::shared_ptr<Expression>> args)
        : func_(func), args_(args) {}
    
    Value evaluate(Interpreter* interp) override {
        std::vector<Value> argValues;
        for (auto& arg : args_) {
            argValues.push_back(arg->evaluate(interp));
        }
        
        switch (func_) {
            case TokenType::SIN: return funcSin(argValues[0]);
            case TokenType::COS: return funcCos(argValues[0]);
            case TokenType::TAN: return funcTan(argValues[0]);
            case TokenType::ATN: return funcAtn(argValues[0]);
            case TokenType::EXP: return funcExp(argValues[0]);
            case TokenType::LOG: return funcLog(argValues[0]);
            case TokenType::SQR: return funcSqr(argValues[0]);
            case TokenType::ABS: return funcAbs(argValues[0]);
            case TokenType::INT: return funcInt(argValues[0]);
            case TokenType::SGN: return funcSgn(argValues[0]);
            case TokenType::RND: return funcRnd(argValues[0]);
            case TokenType::LEN: return funcLen(argValues[0]);
            case TokenType::VAL: return funcVal(argValues[0]);
            case TokenType::ASC: return funcAsc(argValues[0]);
            case TokenType::CHR: return funcChr(argValues[0]);
            case TokenType::LEFT: return funcLeft(argValues[0], argValues[1]);
            case TokenType::RIGHT: return funcRight(argValues[0], argValues[1]);
            case TokenType::MID: return funcMid(argValues[0], argValues[1], argValues[2]);
            case TokenType::STR: return funcStr(argValues[0]);
            default: return Value(0.0);
        }
    }
private:
    TokenType func_;
    std::vector<std::shared_ptr<Expression>> args_;
};

// Statement classes
class PrintStmt : public Statement {
public:
    PrintStmt(std::vector<std::shared_ptr<Expression>> exprs, std::vector<bool> newlines)
        : exprs_(exprs), newlines_(newlines) {}
    
    void execute(Interpreter* interp) override {
        for (size_t i = 0; i < exprs_.size(); ++i) {
            Value val = exprs_[i]->evaluate(interp);
            std::cout << val.getString();
            if (i < newlines_.size() && newlines_[i]) {
                std::cout << "\n";
            }
        }
        if (newlines_.empty() || newlines_.back()) {
            std::cout << "\n";
        }
    }
private:
    std::vector<std::shared_ptr<Expression>> exprs_;
    std::vector<bool> newlines_;
};

class LetStmt : public Statement {
public:
    LetStmt(const std::string& var, std::shared_ptr<Expression> expr)
        : var_(var), expr_(expr) {}
    
    void execute(Interpreter* interp) override {
        Value val = expr_->evaluate(interp);
        interp->getVariables().setVariable(var_, val);
    }
private:
    std::string var_;
    std::shared_ptr<Expression> expr_;
};

class EndStmt : public Statement {
public:
    void execute(Interpreter* interp) override {
        interp->endProgram();
    }
};

// Parser implementation
Parser::Parser() {}

std::vector<std::shared_ptr<Statement>> Parser::parse(const std::vector<Token>& tokens) {
    std::vector<std::shared_ptr<Statement>> statements;
    size_t pos = 0;
    
    while (pos < tokens.size()) {
        // Skip newlines
        if (tokens[pos].type == TokenType::NEWLINE) {
            pos++;
            continue;
        }
        
        // Check for statement separator
        if (tokens[pos].type == TokenType::COLON) {
            pos++;
            continue;
        }
        
        auto stmt = parseStatement(tokens, pos);
        if (stmt) {
            statements.push_back(stmt);
        }
    }
    
    return statements;
}

std::shared_ptr<Statement> Parser::parseStatement(const std::vector<Token>& tokens, size_t& pos) {
    if (pos >= tokens.size()) return nullptr;
    
    Token& token = const_cast<Token&>(tokens[pos]);
    
    switch (token.type) {
        case TokenType::PRINT: return parsePrint(tokens, pos);
        case TokenType::LET: return parseLetOrAssignment(tokens, pos);
        case TokenType::END: pos++; return std::make_shared<EndStmt>();
        case TokenType::IDENTIFIER: return parseLetOrAssignment(tokens, pos);
        default: pos++; return nullptr;
    }
}

std::shared_ptr<Statement> Parser::parsePrint(const std::vector<Token>& tokens, size_t& pos) {
    pos++; // Skip PRINT
    
    std::vector<std::shared_ptr<Expression>> exprs;
    std::vector<bool> newlines;
    
    while (pos < tokens.size() && tokens[pos].type != TokenType::COLON && 
           tokens[pos].type != TokenType::NEWLINE) {
        auto expr = parseExpression(tokens, pos);
        exprs.push_back(expr);
        
        if (pos < tokens.size()) {
            if (tokens[pos].type == TokenType::SEMICOLON) {
                newlines.push_back(false);
                pos++;
            } else if (tokens[pos].type == TokenType::COMMA) {
                newlines.push_back(false);
                pos++;
            } else {
                newlines.push_back(true);
                break;
            }
        }
    }
    
    return std::make_shared<PrintStmt>(exprs, newlines);
}

std::shared_ptr<Statement> Parser::parseLetOrAssignment(const std::vector<Token>& tokens, size_t& pos) {
    if (tokens[pos].type == TokenType::LET) {
        pos++; // Skip LET
    }
    
    if (pos >= tokens.size() || tokens[pos].type != TokenType::IDENTIFIER) {
        throw std::runtime_error("SYNTAX ERROR");
    }
    
    std::string varName = tokens[pos].text;
    pos++;
    
    if (pos >= tokens.size() || tokens[pos].type != TokenType::EQUAL) {
        throw std::runtime_error("SYNTAX ERROR");
    }
    pos++;
    
    auto expr = parseExpression(tokens, pos);
    return std::make_shared<LetStmt>(varName, expr);
}

std::shared_ptr<Expression> Parser::parseExpression(const std::vector<Token>& tokens, size_t& pos) {
    return parseOrExpression(tokens, pos);
}

std::shared_ptr<Expression> Parser::parseOrExpression(const std::vector<Token>& tokens, size_t& pos) {
    auto left = parseAndExpression(tokens, pos);
    
    while (pos < tokens.size() && tokens[pos].type == TokenType::OR) {
        pos++;
        auto right = parseAndExpression(tokens, pos);
        left = std::make_shared<BinaryExpr>(left, TokenType::OR, right);
    }
    
    return left;
}

std::shared_ptr<Expression> Parser::parseAndExpression(const std::vector<Token>& tokens, size_t& pos) {
    auto left = parseRelationalExpression(tokens, pos);
    
    while (pos < tokens.size() && tokens[pos].type == TokenType::AND) {
        pos++;
        auto right = parseRelationalExpression(tokens, pos);
        left = std::make_shared<BinaryExpr>(left, TokenType::AND, right);
    }
    
    return left;
}

std::shared_ptr<Expression> Parser::parseNotExpression(const std::vector<Token>& tokens, size_t& pos) {
    return parseRelationalExpression(tokens, pos);
}

std::shared_ptr<Expression> Parser::parseRelationalExpression(const std::vector<Token>& tokens, size_t& pos) {
    auto left = parseAdditiveExpression(tokens, pos);
    
    if (pos < tokens.size()) {
        TokenType op = tokens[pos].type;
        if (op == TokenType::EQUAL || op == TokenType::NOT_EQUAL ||
            op == TokenType::LESS || op == TokenType::GREATER ||
            op == TokenType::LESS_EQUAL || op == TokenType::GREATER_EQUAL) {
            pos++;
            auto right = parseAdditiveExpression(tokens, pos);
            return std::make_shared<BinaryExpr>(left, op, right);
        }
    }
    
    return left;
}

std::shared_ptr<Expression> Parser::parseAdditiveExpression(const std::vector<Token>& tokens, size_t& pos) {
    auto left = parseMultiplicativeExpression(tokens, pos);
    
    while (pos < tokens.size()) {
        TokenType op = tokens[pos].type;
        if (op == TokenType::PLUS || op == TokenType::MINUS) {
            pos++;
            auto right = parseMultiplicativeExpression(tokens, pos);
            left = std::make_shared<BinaryExpr>(left, op, right);
        } else {
            break;
        }
    }
    
    return left;
}

std::shared_ptr<Expression> Parser::parseMultiplicativeExpression(const std::vector<Token>& tokens, size_t& pos) {
    auto left = parsePowerExpression(tokens, pos);
    
    while (pos < tokens.size()) {
        TokenType op = tokens[pos].type;
        if (op == TokenType::MULTIPLY || op == TokenType::DIVIDE || op == TokenType::MOD) {
            pos++;
            auto right = parsePowerExpression(tokens, pos);
            left = std::make_shared<BinaryExpr>(left, op, right);
        } else {
            break;
        }
    }
    
    return left;
}

std::shared_ptr<Expression> Parser::parseUnaryExpression(const std::vector<Token>& tokens, size_t& pos) {
    return parsePowerExpression(tokens, pos);
}

std::shared_ptr<Expression> Parser::parsePowerExpression(const std::vector<Token>& tokens, size_t& pos) {
    auto left = parsePrimaryExpression(tokens, pos);
    
    if (pos < tokens.size() && tokens[pos].type == TokenType::POWER) {
        pos++;
        auto right = parsePowerExpression(tokens, pos);
        left = std::make_shared<BinaryExpr>(left, TokenType::POWER, right);
    }
    
    return left;
}

std::shared_ptr<Expression> Parser::parsePrimaryExpression(const std::vector<Token>& tokens, size_t& pos) {
    if (pos >= tokens.size()) {
        throw std::runtime_error("SYNTAX ERROR");
    }
    
    const Token& token = tokens[pos];
    
    if (token.type == TokenType::NUMBER) {
        pos++;
        return std::make_shared<LiteralExpr>(token.value);
    }
    
    if (token.type == TokenType::STRING) {
        pos++;
        return std::make_shared<LiteralExpr>(token.value);
    }
    
    if (token.type == TokenType::IDENTIFIER) {
        pos++;
        return std::make_shared<VariableExpr>(token.text);
    }
    
    if (token.type == TokenType::LPAREN) {
        pos++;
        auto expr = parseExpression(tokens, pos);
        if (pos >= tokens.size() || tokens[pos].type != TokenType::RPAREN) {
            throw std::runtime_error("SYNTAX ERROR: MISSING )");
        }
        pos++;
        return expr;
    }
    
    // Built-in functions
    if (token.type == TokenType::SIN || token.type == TokenType::COS ||
        token.type == TokenType::TAN || token.type == TokenType::ATN ||
        token.type == TokenType::EXP || token.type == TokenType::LOG ||
        token.type == TokenType::SQR || token.type == TokenType::ABS ||
        token.type == TokenType::INT || token.type == TokenType::SGN ||
        token.type == TokenType::RND || token.type == TokenType::LEN ||
        token.type == TokenType::VAL || token.type == TokenType::ASC) {
        TokenType func = token.type;
        pos++;
        
        if (pos >= tokens.size() || tokens[pos].type != TokenType::LPAREN) {
            throw std::runtime_error("SYNTAX ERROR");
        }
        pos++;
        
        std::vector<std::shared_ptr<Expression>> args;
        args.push_back(parseExpression(tokens, pos));
        
        if (pos >= tokens.size() || tokens[pos].type != TokenType::RPAREN) {
            throw std::runtime_error("SYNTAX ERROR: MISSING )");
        }
        pos++;
        
        return std::make_shared<FunctionCallExpr>(func, args);
    }
    
    throw std::runtime_error("SYNTAX ERROR");
}

std::shared_ptr<Statement> Parser::parseInput(const std::vector<Token>& tokens, size_t& pos) {
    return nullptr;
}

std::shared_ptr<Statement> Parser::parseIf(const std::vector<Token>& tokens, size_t& pos) {
    return nullptr;
}

std::shared_ptr<Statement> Parser::parseGoto(const std::vector<Token>& tokens, size_t& pos) {
    return nullptr;
}

std::shared_ptr<Statement> Parser::parseGosub(const std::vector<Token>& tokens, size_t& pos) {
    return nullptr;
}

std::shared_ptr<Statement> Parser::parseFor(const std::vector<Token>& tokens, size_t& pos) {
    return nullptr;
}

std::shared_ptr<Statement> Parser::parseNext(const std::vector<Token>& tokens, size_t& pos) {
    return nullptr;
}

std::shared_ptr<Statement> Parser::parseDim(const std::vector<Token>& tokens, size_t& pos) {
    return nullptr;
}

std::shared_ptr<Statement> Parser::parseData(const std::vector<Token>& tokens, size_t& pos) {
    return nullptr;
}

std::shared_ptr<Statement> Parser::parseRead(const std::vector<Token>& tokens, size_t& pos) {
    return nullptr;
}

std::shared_ptr<Statement> Parser::parseDef(const std::vector<Token>& tokens, size_t& pos) {
    return nullptr;
}

bool Parser::match(const std::vector<Token>& tokens, size_t pos, TokenType type) const {
    return pos < tokens.size() && tokens[pos].type == type;
}

bool Parser::isAtEnd(const std::vector<Token>& tokens, size_t pos) const {
    return pos >= tokens.size();
}
