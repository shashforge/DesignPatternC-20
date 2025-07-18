/*
1 Why a “prototype” at all?

| Pain-point                                                                                               | Prototype solution                                                                                                   |
| -------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------- |
| **Object creation is expensive or complicated** (deep graphs, DB look-ups, parsing a huge config, etc.). | Cache **one fully-built “prototype”** and ask it to `clone()` itself, bypassing the heavy construction work.         |
| **Class of object is chosen at run-time** (reading config, reacting to user input).                      | Store heterogeneous prototypes in a **registry** indexed by a key string/ID; the client code can stay type-agnostic. |
| **You need copies that capture run-time state** (e.g. game entities whose current hit-points matter).    | Cloning preserves the exact current state, whereas Factory only builds from static parameters.                       |

Trade-offs & best practices
| ✅ Good fit when…                                           | ⚠️ Watch out for…                                                              |
| ---------------------------------------------------------- | ------------------------------------------------------------------------------ |
| Construction cost >> copy cost.                            | **Object slicing** if you forget virtual `clone()`.                            |
| Clients need objects & variants unknown at compile-time.   | Correctness of deep copy: shared resources, IDs, listeners may need reseating. |
| You must preserve run-time state.                          | Memory bloat if prototypes are heavyweight and never reused.                   |
| You want to decouple instantiation logic from client code. | A registry can turn into a global object; consider dependency injection.       |

-Prototype vs. other creational patterns
Factory Method / Abstract Factory – create new objects from scratch based on parameters; cannot easily capture current state.
Builder – step-by-step construction for very complex objects; costlier for each instance than cloning.
Singleton – one shared instance; Prototype wants many independent instances.

*/

//Four eras of C++ implementation
// C++98 – manual memory, raw pointers

// Prototype base
class Shape {
public:
    virtual ~Shape() {}
    virtual Shape* clone() const = 0;            // mandatory
    virtual void draw() const = 0;
};

// ConcretePrototype
class Circle : public Shape {
    double r_;
public:
    Circle(double r) : r_(r) {}
    Circle(const Circle& other) : r_(other.r_) {} // deep copy if needed
    Shape* clone() const override { return new Circle(*this); }
    void draw() const override { /*...*/ }
};

class Square : public Shape { /* identical idea */ };

// Registry & client
#include <map>
Shape* make_shape(const std::string& key)
{
    static std::map<std::string, Shape*> reg {
        { "circle", new Circle{1.0} },
        { "square", new Square{2.0} }
    };
    return reg.at(key)->clone();                 // caller owns
}


//C++17 – smart pointers & type-safe memory
#include <memory>
class Shape {
public:
    virtual ~Shape() = default;
    virtual std::unique_ptr<Shape> clone() const = 0;
    virtual void draw() const = 0;
};

class Circle final : public Shape {
    double r_;
public:
    explicit Circle(double r) : r_{r} {}
    std::unique_ptr<Shape> clone() const override {
        return std::make_unique<Circle>(*this);   // deep copy via copy-ctor
    }
    void draw() const override { /* ... */ }
};

// Registry powered by smart pointers
#include <unordered_map>
std::unique_ptr<Shape> make_shape(const std::string& key)
{
    static const std::unordered_map<std::string, std::unique_ptr<Shape>> reg {
        { "circle", std::make_unique<Circle>(1.0) },
        { "square", std::make_unique<Square>(2.0) }
    };
    return reg.at(key)->clone();                  // ownership correctly moved
}

/*Highlights

std::unique_ptr gives exception-safe ownership.
final and default/override clarify intent
std::unordered_map is faster than std::map for look-ups.*/

//C++20 – concepts & cleaner APIs
#include <concepts>
template<typename T>
concept Cloneable =
    std::derived_from<T, Shape> &&
    requires(const T& t) { { t.clone() } -> std::same_as<std::unique_ptr<Shape>>; };

class Triangle : public Shape { /* …same idea… */ };

template<Cloneable S>
std::unique_ptr<Shape> register_proto(std::string_view key, S proto)
{
    auto& reg = prototypes();                     // hidden singleton
    reg.emplace(key, std::make_unique<S>(std::move(proto)));
    return reg.at(key)->clone();
}

auto clone_any(std::string_view key)
{
    auto& reg = prototypes();
    return reg.at(key)->clone();                  // still runtime-polymorphic
}
/*Highlights
Concept Cloneable gives compiler-checked guarantee that each ConcretePrototype really returns the right type.
Ranges algorithms (std::ranges::for_each) can iterate over the registry with clarity.*/

//C++23 – deducing this & more expressive syntax
struct Shape {
    // “deducing-this” lets clone be a free template that deduces the dynamic type
    auto clone(this const Shape& self) const
        -> std::unique_ptr<Shape>
    {
        return std::unique_ptr<Shape>(self.clone_impl());
    }
private:
    virtual Shape* clone_impl() const = 0;
};

struct Hexagon : Shape {
    int sides{6};
private:
    Hexagon* clone_impl() const override { return new Hexagon(*this); }
};

// Client side
auto h1 = Hexagon{};
auto h2 = h1.clone();   // nice value-like syntax, no ‘virtual’ in user code

/*this as explicit parameter eliminates boiler-plate of writing Hexagon::clone(); one generic wrapper suffices.
Default member initializers & auto return types trim syntax noise further.
Combined with C++23’s improved std::expected or std::out_ptr, error handling around cloning can be even safer, though not shown here.*/






