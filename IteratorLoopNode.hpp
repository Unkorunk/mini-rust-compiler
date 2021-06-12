#pragma once

#include "BlockNode.hpp"
#include "PatternNodes.hpp"

class IteratorLoopNode : public ExpressionNode {
public:
    IteratorLoopNode(
        std::unique_ptr<PatternNode> &&pattern, std::unique_ptr<ExpressionNode> &&expression,
        std::unique_ptr<BlockNode> &&block);

    void Visit(SyntaxTreeVisitor *visitor) const override;

    const PatternNode *GetPattern() const;
    const ExpressionNode *GetExpression() const;
    const BlockNode *GetBlock() const;

private:
    std::unique_ptr<PatternNode> pattern_;
    std::unique_ptr<ExpressionNode> expression_;
    std::unique_ptr<BlockNode> block_;
};
