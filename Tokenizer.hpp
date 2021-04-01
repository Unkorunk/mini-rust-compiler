#pragma once

#include <sstream>
#include <array>
#include <fstream>
#include <vector>
#include <stack>
#include <algorithm>
#include <map>

#include "Token.hpp"
#include "KeywordManager.hpp"
#include "Punctuation.hpp"
#include "InputStream.hpp"
#include "TokenizerHelper.hpp"

class Tokenizer {
public:
    explicit Tokenizer(std::ifstream *stream);

    bool HasNext() const;
    Token Next();

private:
    InputStream stream_;

    void SkipWhitespace();
    void SkipLineComment();
    void SkipMultilineComment();

    Token TokenizeIdentifierOrKeyword();
    // character and string literals
    Token TokenizeCharacter();
    Token TokenizeString();
    Token TokenizeRawString();
    // byte and byte string literals
    Token TokenizeByte();
    Token TokenizeByteString();
    Token TokenizeRawByteString();
    // number literals
    Token TokenizeNumber();
    // TODO lifetimes and loop labels

    Token MakeToken(Token::Type type) {
        return Token(type, stream_.GetStartLine(), stream_.GetStartColumn(), stream_.GetCurrentLine(), stream_.GetCurrentColumn());
    }
    Token MakeToken(TokenValue value, Token::Type type) {
        return Token(value, type, stream_.GetStartLine(), stream_.GetStartColumn(), stream_.GetCurrentLine(), stream_.GetCurrentColumn());
    }
    Token MakeError(std::string error_text) {
        return MakeToken(TokenValue(error_text), Token::Type::kError);
    }
    Token MakeLiteral(TokenValue value) {
        return MakeToken(value, Token::Type::kLiteral);
    }
    Token MakeIdentifier(std::string name) {
        return MakeToken(TokenValue(name), Token::Type::kIdentifier);
    }

    int curly_balance = 0, square_balance = 0, round_balance = 0;

    template <typename T>
    static bool TryParse(const std::vector<int8_t>& digits, T *result) {
        *result = 0;
        for (int8_t digit : digits) {
            T tmp = *result * 10;
            if (tmp / 10 != *result) {
                return false;
            }
            if (tmp > std::numeric_limits<T>::max() - digit) {
                return false;
            }
            *result = tmp + digit;
        }
        return true;
    }

    template <class Type, class... Types>
    static bool TryParse(const std::vector<int8_t>& digits, TokenValue *result) {
        Type tmp1;
        if (!TryParse<Type>(digits, &tmp1)) {
            return TryParse<Types...>(digits, result);
        }
        *result = tmp1;
        return true;
    }

    template <>
    static bool TryParse(const std::vector<int8_t>& digits, TokenValue *result) {
        return false;
    }

    const std::vector<Punctuation> punctuation_ = {
        {'\'', std::bind(&Tokenizer::TokenizeCharacter, this)},
        {'"', std::bind(&Tokenizer::TokenizeString, this)},
        {'r', std::bind(&Tokenizer::TokenizeRawString, this)},
        {'b', std::bind(&Tokenizer::MakeError, this, "TODO"), {
            {'\'', std::bind(&Tokenizer::TokenizeByte, this)},
            {'"', std::bind(&Tokenizer::TokenizeByteString, this)},
            {'r', std::bind(&Tokenizer::TokenizeRawByteString, this)}
        }},
        {'-', Token::Type::kMinus, {
            {'=', Token::Type::kMinusEq},
            {'>', Token::Type::kRArrow}
        }},
        {'&', Token::Type::kAnd, {
            {'=', Token::Type::kAndEq},
            {'&', Token::Type::kAndAnd}
        }},
        {'|', Token::Type::kOr, {
            {'=', Token::Type::kOrEq},
            {'|', Token::Type::kOrOr}
        }},
        {'<', Token::Type::kLt, {
            {'<', Token::Type::kShl, {
                {'=', Token::Type::kShlEq}
            }},
            {'=', Token::Type::kLe}
        }},
        {'>', Token::Type::kGt, {
            {'>', Token::Type::kShr, {
                {'=', Token::Type::kShrEq}
            }},
            {'=', Token::Type::kGe}
        }},
        {'=', Token::Type::kEq, {
            {'=', Token::Type::kEqEq},
            {'>', Token::Type::kFatArrow}
        }},
        {'.', Token::Type::kDot, {
            {'.', Token::Type::kDotDot, {
                {'.', Token::Type::kDotDotDot},
                {'=', Token::Type::kDotDotEq}
            }}
        }},
        {':', Token::Type::kColon, {
            {':', Token::Type::kPathSep}
        }},
        {'/', Token::Type::kSlash, {
            {'=', Token::Type::kSlashEq},
            {'/', [this]() { SkipLineComment(); return Next(); }},
            {'*', [this]() { SkipMultilineComment(); return Next(); }}
        }},
        {'@', Token::Type::kAt},
        {'_', Token::Type::kUnderscore},
        {',', Token::Type::kComma},
        {';', Token::Type::kSemi},
        {'#', Token::Type::kPound},
        {'$', Token::Type::kDollar},
        {'?', Token::Type::kQuestion},
        {'+', Token::Type::kPlus, { {'=', Token::Type::kPlusEq} }},
        {'*', Token::Type::kStar, { {'=', Token::Type::kStarEq} }},
        {'%', Token::Type::kPercent, { {'=', Token::Type::kPercentEq} }},
        {'^', Token::Type::kCaret, { {'=', Token::Type::kCaretEq} }},
        {'!', Token::Type::kNot, { {'=', Token::Type::kNe} }},
        {'{', std::bind(&Tokenizer::TokenizeOpenBr, this, Token::Type::kOpenCurlyBr, &curly_balance)},
        {'}', std::bind(&Tokenizer::TokenizeCloseBr, this, Token::Type::kCloseCurlyBr, &curly_balance)},
        {'[', std::bind(&Tokenizer::TokenizeOpenBr, this, Token::Type::kOpenSquareBr, &square_balance)},
        {']', std::bind(&Tokenizer::TokenizeCloseBr, this, Token::Type::kCloseSquareBr, &square_balance)},
        {'(', std::bind(&Tokenizer::TokenizeOpenBr, this, Token::Type::kOpenRoundBr, &round_balance)},
        {')', std::bind(&Tokenizer::TokenizeCloseBr, this, Token::Type::kCloseRoundBr, &round_balance)}
    };

    Token TokenizeOpenBr(Token::Type type, int *balance) {
        (*balance)++;
        return MakeToken(type);
    }

    Token TokenizeCloseBr(Token::Type type, int *balance) {
        if (*balance == 0) {
            return MakeError("TODO");
        }
        return MakeToken(type);
    }
};
