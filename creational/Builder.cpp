/*Builder Pattern — detailed walkthrough
The Builder pattern lets you construct a complex object step-by-step while keeping the final representation immutable and the construction logic readable.
Instead of a constructor with a dozen parameters (the telescoping constructor problem), you expose a small builder object that gradually gathers the pieces and finally emits the finished product.

Key roles

Role	Responsibility
Product	The object being assembled (usually immutable once built).
Builder interface	Declares fluent “setter”-style steps and a build()/get() to finish.
Concrete Builder	Holds the partially-built state and implements the interface.
Director (optional)	Orchestrates a standard recipe (“make a deluxe pizza”) by calling the builder in a fixed order.

Why use it?

✅ Eliminates monstrous constructors / long parameter lists.

✅ Makes required vs. optional parts explicit.

✅ Encourages immutable end objects (all mutation happens inside the builder).

✅ Lets you reuse the same construction steps to create multiple “flavors” of a product or even entirely different representations.

Classic fluent Builder in C++17 */
// Compile with:  g++ -std=c++17 builder17.cpp
#include <iostream>
#include <string>
#include <vector>

class Pizza {
public:
    struct Specs {
        std::string crust;
        int         size_cm = 32;
        std::vector<std::string> toppings;
    };

    // Expose a read-only view
    const Specs& specs() const { return specs_; }

    /* ---------- Builder ---------- */
    class Builder {
    public:
        Builder& crust(std::string c)               { specs_.crust = std::move(c); return *this; }
        Builder& size(int cm)                       { specs_.size_cm = cm;         return *this; }
        Builder& addTopping(std::string t)          { specs_.toppings.push_back(std::move(t)); return *this; }
        Pizza build() const                         { return Pizza{specs_}; }

    private:
        Specs specs_;
    };

private:
    explicit Pizza(const Specs& s) : specs_{s} {}  // private ctor
    Specs specs_;
};

/* ---------- Usage ---------- */
int main() {
    Pizza margherita = Pizza::Builder{}
                           .crust("Neapolitan")
                           .size(30)
                           .addTopping("Tomato")
                           .addTopping("Mozzarella")
                           .build();

    std::cout << "Crust: " << margherita.specs().crust
              << ", size: " << margherita.specs().size_cm << " cm\n";
}


//constexpr-friendly Builder in C++20
// Compile with:  g++ -std=c++20 builder20.cpp
#include <array>
#include <cassert>
#include <iostream>
#include <string_view>
#include <vector>
#include <ranges>

template<int Min, int Max>
concept InRange = requires(int v) { (v >= Min && v <= Max); };

class Pizza {
public:
    struct Specs {
        std::string_view crust;
        int size_cm = 32;
        std::vector<std::string_view> toppings;
    };

    constexpr const Specs& specs() const { return specs_; }

    /* ---------- Builder ---------- */
    class Builder {
    public:
        constexpr Builder& crust(std::string_view c)  { specs_.crust = c; return *this; }
        template<InRange<20, 60> Size>
        constexpr Builder& size(Size cm)              { specs_.size_cm = cm; return *this; }

        template<std::ranges::input_range R>
            requires std::convertible_to<std::ranges::range_value_t<R>, std::string_view>
        constexpr Builder& toppings(R&& range) {
            specs_.toppings.assign(std::ranges::begin(range), std::ranges::end(range));
            return *this;
        }

        constexpr Pizza build() const { return Pizza{specs_}; }

    private:
        Specs specs_;
    };

private:
    constexpr explicit Pizza(const Specs& s) : specs_{s} {}
    Specs specs_;
};

/* ---------- Usage ---------- */
consteval Pizza hawaiian() {
    using namespace std::string_view_literals;
    return Pizza::Builder{}
        .crust("Pan")
        .size(40)
        .toppings(std::array{ "Ham"sv, "Pineapple"sv })
        .build();
}

int main() {
    constexpr auto p = hawaiian();
    static_assert(p.specs().toppings.size() == 2);
    std::cout << "C++20 constexpr pizza size: " << p.specs().size_cm << " cm\n";
}


//deducing-this” fluent builder in C++23
//C++23 introduces deducing this and explicit object parameters, enabling free-function-style fluent calls without boilerplate return *this;.
//We’ll also mark build() consteval to guarantee the product is finished during compilation (but still callable at runtime).
// Compile with:  g++ -std=c++23 builder23.cpp
#include <iostream>
#include <print>
#include <string_view>
#include <vector>

class Pizza {
public:
    struct Specs {
        std::string_view crust;
        int size_cm = 32;
        std::vector<std::string_view> toppings;
    };

    const Specs& specs() const { return specs_; }

    /* ---------- Builder using deducing-this ---------- */
    class Builder {
    public:
        // deducing-this: first parameter is the object being invoked
        auto crust(this Builder self, std::string_view c) {
            self.specs_.crust = c;
            return self;
        }
        auto size(this Builder self, int cm) {
            self.specs_.size_cm = cm;
            return self;
        }
        auto add(this Builder self, std::string_view topping) {
            self.specs_.toppings.push_back(topping);
            return self;
        }
        consteval Pizza build(this const Builder self) {
            // simple validation
            if (self.specs_.crust.empty())
                throw "Crust type required!";
            return Pizza{self.specs_};
        }
    private:
        Specs specs_;
    };

private:
    explicit Pizza(const Specs& s) : specs_{s} {}
    Specs specs_;
};

/* ---------- Usage ---------- */
int main() {
    using namespace std::string_view_literals;

    constexpr Pizza quattro =
        Pizza::Builder{}
            .crust("Roman")
            .size(33)
            .add("Mozzarella")
            .add("Parmesan")
            .add("Gorgonzola")
            .add("Ricotta")
            .build();

    std::println("C++23 pizza toppings: {}", quattro.specs().toppings.size());
}

