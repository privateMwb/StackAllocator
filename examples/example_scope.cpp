// StackScope example.
//
// Demonstrates:
// - Automatic state restoration with StackScope
// - Nested StackScope instances
// - StackScope with object lifetime management

#include "example_helper.h"

using namespace AllocatorPro;

struct Node {
    int   value = 0;
    float score = 0.0f;
};

int main() {
    mainTitle("\nScope Examples");
    borderLine();

    // Automatic state restoration.
    setTitle("Scope Restoration");

    Stack s{1024};
    (void)s.allocate(128);

    std::cout << "Used before scope : " << s.used() << "\n";

    {
        StackScope scope{s};
        (void)s.allocate(256);
        std::cout << "Used inside scope : " << s.used() << "\n";
    }

    std::cout << "Used after scope  : " << s.used() << "\n\n";

    // Nested StackScope instances.
    setTitle("Nested Scopes");

    s.reset();

    {
        StackScope outer{s};
        (void)s.allocate(128);
        std::cout << "Used after outer alloc  : " << s.used() << "\n";

        {
            StackScope inner{s};
            (void)s.allocate(128);
            std::cout << "Used after inner alloc  : " << s.used() << "\n";
        }

        std::cout << "Used after inner scope  : " << s.used() << "\n";
    }

    std::cout << "Used after outer scope  : " << s.used() << "\n\n";

    // StackScope with object lifetime management.
    setTitle("Scope With Object Lifecycle");

    s.reset();

    {
        StackScope scope{s};
        Node* node  = s.create<Node>();
        node->value = 42;
        node->score = 3.14f;

        std::cout << "Node value       : " << node->value << "\n";
        std::cout << "Node score       : " << node->score << "\n";
        std::cout << "Used             : " << s.used()    << "\n";

        s.destroy(node);
    }

    std::cout << "Used after scope : " << s.used() << "\n";

    borderLine();
    std::cout << "\n";
    return 0;
}

