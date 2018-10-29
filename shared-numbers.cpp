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
 * This implementation uses shared pointers to pass references around in the call stack of
 * solve.
 */

#include <iostream>
#include <vector>
#include <memory>
#include <array>
#include <string>
#include <algorithm>
#include <chrono>

// functions for all operations
using Operation = int(*)(int, int);

int add(int const a, int const b)
{
    return a + b;
}

int sub(int const a, int const b)
{
    return a - b;
}

int mul(int const a, int const b)
{
    return a * b;
}

// div is already used
int rat(int const a, int const b)
{
    return a / b;
}

std::array ops{add, sub, mul, rat};

// turn operation into string
std::string str(Operation const &op)
{
    if (op == add) {
        return std::string("+");
    }
    if (op == sub) {
        return std::string("-");
    }
    if (op == mul) {
        return std::string("*");
    }
    if (op == rat) {
        return std::string("/");
    }
    return std::string("?");
}


// abstract base for nodes
struct Node
{
    virtual ~Node() = default;
    virtual int eval() = 0;
    virtual std::string str() = 0;
};
// yeah, yeah, it is easiest to use here...
using NodePtr = std::shared_ptr<Node>;

// just a number
struct Number : Node
{
    int number;

    explicit Number(int n) : number{n} { }
    ~Number() override = default;

    int eval() override
    {
        return number;
    }

    std::string str() override
    {
        return std::to_string(number);
    }
};

// binary operation
struct Binary : Node
{
private:
    // memoise the value
    int value_ = invalid;
    // memoise string
    std::string s_{};

    // cannot have negative number, use -1 as sentinel
    constexpr static int invalid = -1;

public:
    Operation op;
    NodePtr a, b;  // operands

    explicit Binary(Operation op, NodePtr a, NodePtr b)
        : op{op}, a{a}, b{b} { }
    ~Binary() override = default;

    int eval() override
    {
        if (value_ == invalid) {
            value_ = op(a->eval(), b->eval());
        }
        return value_;
    }

    std::string str() override
    {
        if (s_.empty()) {
            s_ = '('+a->str()+' '+::str(op)+' '+b->str()+')';
        }
        return s_;
    }
};

// turn array into vector of number nodes
template <size_t N>
auto toNodes(std::array<int, N> const &numbers)
{
    std::vector<NodePtr> numberNodes;
    for (int n : numbers) {
        numberNodes.emplace_back(std::make_shared<Number>(n));
    }
    return numberNodes;
}

// print a collection of nodes
template <typename NS>
void printNodes(NS const &ns)
{
    for (auto &node : ns)
        std::cout << node->str() << '[' << node->eval() << ']' << "  ";
    std::cout << '\n';
}

// Solve the game recursively.
// Use a set of starting nodes and try all binary combinations.
// Recurse with a vector with two nodes erased and one extra node for the new operation.
std::vector<NodePtr> solve(std::vector<NodePtr> const &startNodes,
                           int const target)
{
    std::vector<NodePtr> solutions;

    for (auto op : ops) {
        for (size_t i = 0; i < startNodes.size(); ++i) {
            // first operand to try
            NodePtr const &nodei = startNodes[i];
            // new vector without nodei
            std::vector<NodePtr> auxNodes(startNodes);
            auxNodes.erase(std::begin(auxNodes)+i);

            for (size_t j = 0; j < auxNodes.size(); ++j) {
                // second operand to try
                NodePtr const &nodej = auxNodes[j];

                // only try every pair once: the order that is ok for sub
                if (nodei->eval() <= nodej->eval()) continue;
                // skip divisions with remainder
                if (op == rat and nodei->eval() % nodej->eval() != 0) continue;

                // new vector without nodei and nodej
                std::vector<NodePtr> newNodes(auxNodes);
                newNodes.erase(std::begin(newNodes)+j);

                // make a new binary node
                auto n = std::make_shared<Binary>(op, nodei, nodej);
                if (n->eval() == target) {
                    solutions.emplace_back(n);
                }
                newNodes.emplace_back(std::move(n));

                // printNodes(newNodes);

                // recurse if enough nodes left
                if (std::size(newNodes) > 1) {
                    auto const sols = solve(newNodes, target);
                    std::copy(std::begin(sols), std::end(sols), std::back_inserter(solutions));
                }
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

    // solve
    auto startTimeSol = std::chrono::steady_clock::now();
    auto solutions = solve(numberNodes, target);
    auto endTimeSol = std::chrono::steady_clock::now();
    std::cout << '\n';

    // erase all duplicates
    auto startTimeUnique = std::chrono::steady_clock::now();
    std::sort(std::begin(solutions), std::end(solutions),
              [](NodePtr const &n1, NodePtr const &n2) {
                  return n1->str() < n2->str();
              });
    solutions.erase(std::unique(std::begin(solutions), std::end(solutions),
                                [](NodePtr const &n1, NodePtr const &n2) {
                                    return n1->str() == n2->str();
                                }),
                    std::end(solutions));
    auto endTimeUnique = std::chrono::steady_clock::now();

    std::cout << "Solutions:\n";
    for (auto &node : solutions)
        std::cout << node->str() << " [" << node->eval() << "]\n";

    std::cout << '\n';
    std::cout << "Time to solution: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(endTimeSol-startTimeSol).count()
              << "ms\n";
    std::cout << "Time to clean up: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(endTimeUnique-startTimeUnique).count()
              << "ms\n";
}
