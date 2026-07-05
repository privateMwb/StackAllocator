// Stack object lifecycle example.
//
// Demonstrates:
// - Creating and destroying trivial types
// - Creating and destroying non-trivial types
// - Constructing objects with forwarded arguments
// - Explicit object destruction

#include <common/framework.h>

#include <string>

using namespace AllocatorPro;

struct Point {
    float x = 0.0f;
    float y = 0.0f;
};

struct Player {
    int         id    = 0;
    float       speed = 0.0f;
    std::string name;

    explicit Player(int id, float speed, std::string name)
        : id    {id}
        , speed {speed}
        , name  {std::move(name)}
    {}

    ~Player() {
        std::cout << "Player destroyed   : " << name << "\n";
    }
};

static void run_examples() {
    Stack s{1024};

    // Creating and destroying a trivial type.
    setTitle("Trivial Type");

    Point* p  = s.create<Point>();
    p->x      = 10.0f;
    p->y      = 20.0f;

    std::cout << "Point x : " << p->x     << "\n";
    std::cout << "Point y : " << p->y     << "\n";
    std::cout << "Used    : " << s.used() << "\n\n";

    s.destroy(p);

    // Constructing a non-trivial type with arguments.
    setTitle("Non-Trivial Type With Arguments");

    Player* player = s.create<Player>(1, 5.5f, "Rain");

    std::cout << "Player id    : " << player->id    << "\n";
    std::cout << "Player speed : " << player->speed << "\n";
    std::cout << "Player name  : " << player->name  << "\n";
    std::cout << "Used         : " << s.used()      << "\n\n";

    // Explicit object destruction.
    setTitle("Destruction");

    s.destroy(player);
    std::cout << "Used after destroy : " << s.used() << "\n";
}

REGISTER_EXAMPLE_SUITE();
