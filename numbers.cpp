/*
 * Solve the numbers game from the TV show countdown.
 *
 * Find all possible combinations of a set of numbers to
 * get a target number.
 * Only positive integers and operations +, -, *, / (no remainder)
 * are allowed.
 *
 * Constructs a tree of operations, trying out all possible combinations.
 * The solution contains duplicates in terms of associativity.
 *
 * This implementation uses raw pointers with memory managed from the outside of the
 * solve function. It moves references around without counting them.
 */

#include <iostream>
#include <vector>
#include <memory>
#include <array>
#include <string>
#include <algorithm>
#include <chrono>
#include <cassert>

struct Node
{
    enum Kind { val, sum, sub, mul, div };

    Kind kind;

private:
    int value_{invalid_};
    Node *a_{nullptr}, *b_{nullptr};

    // cannot have negative number, use -1 as sentinel
    constexpr static int invalid_ = -1;

public:
    explicit Node(int const number) noexcept
        : kind{Kind::val}, value_{number}
    { }

    explicit Node(Kind const operation,
                  Node *a, Node *b) noexcept
        : kind{operation}, a_{a}, b_{b}
    { }

    int eval() noexcept
    {
        if (value_ == invalid_) {
            switch (kind) {
            case sum:
                value_ = a_->eval() + b_->eval();
                break;
            case sub:
                value_ = a_->eval() - b_->eval();
                break;
            case mul:
                value_ = a_->eval() * b_->eval();
                break;
            case div:
                value_ = a_->eval() / b_->eval();
                break;
            default:
                assert(false);
            }
        }

        return value_;
    }

    Node *a() noexcept
    {
        return a_;
    }

    Node *b() noexcept
    {
        return b_;
    }
};

std::string to_string(Node &node)
{
    switch (node.kind) {
    case Node::Kind::val:
        return std::to_string(node.eval());
    case Node::Kind::sum:
        return '('+to_string(*node.a())+" + "+to_string(*node.b())+')';
    case Node::Kind::sub:
        return '('+to_string(*node.a())+" - "+to_string(*node.b())+')';
    case Node::Kind::mul:
        return '('+to_string(*node.a())+" * "+to_string(*node.b())+')';
    case Node::Kind::div:
        return '('+to_string(*node.a())+" / "+to_string(*node.b())+')';
    }
    return {};
}

std::array ops{Node::Kind::sum, Node::Kind::sub, Node::Kind::mul, Node::Kind::div};


// turn array into vector of number nodes
template <size_t N>
auto toNodes(std::array<int, N> const &numbers)
{
    std::vector<std::unique_ptr<Node>> numberNodes;
    for (int n : numbers) {
        numberNodes.emplace_back(std::make_unique<Node>(n));
    }
    return numberNodes;
}

// print a collection of nodes
template <typename NS>
void printNodes(NS const &ns)
{
    for (auto &node : ns)
        std::cout << to_string(*node) << '[' << node->eval() << ']' << "  ";
    std::cout << '\n';
}

// copy a vector but leave out one element
template <typename IT>
void copyExcept(std::vector<Node*> const &in,
                IT const &pos,
                std::vector<Node*> &out)
{
    out.clear();
    for (auto ita = std::cbegin(in); ita != std::cend(in); ++ita) {
        if (ita != pos) {
            out.emplace_back(*ita);
        }
    }
}


// Solve the game recursively.
// Use a set of starting nodes and try all binary combinations.
// Recurse with a vector with two nodes erased and one extra node for the new operation.
// The node memory must be maintained by the caller.
std::vector<std::string> solve(std::vector<Node*> const &startNodes,
                               int const target)
{
    std::vector<std::string> solutions;
    std::vector<Node*> auxNodes, newNodes;
    auxNodes.reserve(std::size(startNodes)-1);
    newNodes.reserve(std::size(startNodes)-2);

    for (auto ita = std::cbegin(startNodes); ita != std::cend(startNodes); ++ita) {
        // first operand to try
        Node * const nodea = *ita;
        // new vector without nodea
        copyExcept(startNodes, ita, auxNodes);

        for (auto itb = std::cbegin(auxNodes); itb != std::cend(auxNodes); ++itb) {
            // second operand to try
            Node * const nodeb = *itb;

            // only try every pair once: the order that is ok for sub
            if (nodea->eval() <= nodeb->eval()) continue;

            // new vector without nodeb and nodea
            copyExcept(auxNodes, itb, newNodes);

            for (auto op : ops) {
                // skip divisions with remainder
                if (op == Node::Kind::div and nodea->eval() % nodeb->eval() != 0) continue;

                // make a new binary node
                Node opNode(op, nodea, nodeb);
                if (opNode.eval() == target) {
                    solutions.emplace_back(to_string(opNode));
                    // don't 'continue' because we might be able to add zero or multiply by one
                }
                newNodes.emplace_back(&opNode);

                // recurse if enough nodes left
                if (std::size(newNodes) > 1) {
                    auto const sols = solve(newNodes, target);
                    std::copy(std::begin(sols), std::end(sols), std::back_inserter(solutions));
                }

                newNodes.pop_back();
            }
        }
    }

    return solutions;
}

int main()
{
    // the number we want to get
    constexpr int target = 784;
    // the input numbers
    constexpr std::array numbers{100, 50, 9, 5, 2, 4};

    // turn them into nodes
    std::cout << "Numbers:\n";
    auto numberNodes = toNodes(numbers);
    printNodes(numberNodes);
    std::cout << '\n';

    std::vector<Node*> workingArray;
    std::transform(std::begin(numberNodes), std::end(numberNodes),
                   std::back_inserter(workingArray),
                   [](std::unique_ptr<Node> const &ptr) { return ptr.get(); });

    // solve
    auto startTimeSol = std::chrono::steady_clock::now();
    auto solutions = solve(workingArray, target);
    auto endTimeSol = std::chrono::steady_clock::now();
    std::cout << '\n';

    // erase all duplicates
    auto startTimeUnique = std::chrono::steady_clock::now();
    std::sort(std::begin(solutions), std::end(solutions));
    solutions.erase(std::unique(std::begin(solutions), std::end(solutions)),
                    std::end(solutions));
    auto endTimeUnique = std::chrono::steady_clock::now();

    std::cout << "Solutions:\n";
    for (auto &solution : solutions)
        std::cout << solution << '\n';
    std::cout << "There are " << std::size(solutions) << " 'distinct' solutions\n";


    std::cout << '\n';
    std::cout << "Time to solution: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(endTimeSol-startTimeSol).count()
              << "ms\n";
    std::cout << "Time to clean up: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(endTimeUnique-startTimeUnique).count()
              << "ms\n";
}
