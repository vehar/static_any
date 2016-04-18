#include "any.hpp"

#include <gtest/gtest.h>

struct A
{
    explicit A(int i) :
        m_i(i) {}
    int m_i;
};

TEST(any, readme_example)
{
    any<32> a; // on g++ 5.x sizeof(std::string) is 32
    static_assert(sizeof(a) == 32 + sizeof(std::ptrdiff_t), "impossible");

    a = 1234;
    ASSERT_EQ(1234, a.get<int>());

    a = std::string("Hello world");
    ASSERT_EQ(std::string("Hello world"), a.get<std::string>());

    struct A
    {
        explicit A(long i = 0, double d = .0)
         : i_(i), d_(d) {}

        long i_;
        double d_;
    };

    a = A(12, .34);
}

TEST(any, size)
{
    any<16> a;
    ASSERT_EQ(16 + sizeof(std::ptrdiff_t), sizeof(a));
}

struct CallCounter
{
    CallCounter() { default_constructions++; }
    CallCounter(const CallCounter&) { copy_constructions++; }
    CallCounter(CallCounter&&) { move_constructions++; }
    ~CallCounter() { destructions++; }

    static void reset_counters()
    {
        default_constructions = 0;
        copy_constructions = 0;
        move_constructions = 0;
        destructions = 0;
    }

    static int default_constructions;
    static int copy_constructions;
    static int move_constructions;
    static int destructions;
};

int CallCounter::default_constructions = 0;
int CallCounter::copy_constructions = 0;
int CallCounter::move_constructions = 0;
int CallCounter::destructions = 0;

TEST(any, default_constructed_is_empty)
{
    any<16> a;
    ASSERT_TRUE(a.empty());
}

TEST(any, contructed_with_param_non_empty)
{
    any<16> a(77); // will contain integer
    ASSERT_FALSE(a.empty());
}

TEST(any, is_stored_type)
{
    any<16> a(77); // will contain integer
    ASSERT_TRUE(a.is_stored_type<int>());
    ASSERT_FALSE(a.is_stored_type<double>());
}

TEST(any, move_construct)
{
    CallCounter::reset_counters();
    CallCounter counter;
    any<16> a(std::move(counter));

    ASSERT_EQ(1, CallCounter::default_constructions);
    ASSERT_EQ(0, CallCounter::copy_constructions);
    ASSERT_EQ(1, CallCounter::move_constructions);
    ASSERT_EQ(0, CallCounter::destructions);
}

TEST(any, copy_construct)
{
    CallCounter::reset_counters();
    CallCounter counter;
    any<16> a(counter);

    ASSERT_EQ(1, CallCounter::default_constructions);
    ASSERT_EQ(1, CallCounter::copy_constructions);
    ASSERT_EQ(0, CallCounter::move_constructions);
    ASSERT_EQ(0, CallCounter::destructions);
}

TEST(any, destruction)
{
    CallCounter::reset_counters();
    CallCounter counter;
    {
        any<16> a(counter);
    }

    ASSERT_EQ(1, CallCounter::destructions);
}

TEST(any, copy_assignment)
{
    CallCounter::reset_counters();
    CallCounter counter;

    any<16> a(1);
    a = counter;

    ASSERT_EQ(1, CallCounter::default_constructions);
    ASSERT_EQ(1, CallCounter::copy_constructions);
    ASSERT_EQ(0, CallCounter::move_constructions);
}

TEST(any, move_assignment)
{
    CallCounter::reset_counters();
    CallCounter counter;

    any<16> a;
    a = std::move(counter);

    ASSERT_EQ(1, CallCounter::default_constructions);
    ASSERT_EQ(0, CallCounter::copy_constructions);
    ASSERT_EQ(1, CallCounter::move_constructions);
}

TEST(any, reassignment)
{
    CallCounter::reset_counters();
    CallCounter counter;

    any<16> a(counter);
    a = counter;

    ASSERT_EQ(1, CallCounter::default_constructions);
    ASSERT_EQ(2, CallCounter::copy_constructions);
    ASSERT_EQ(0, CallCounter::move_constructions);
    ASSERT_EQ(1, CallCounter::destructions);
}

TEST(any, not_empty_after_assignment)
{
    any<16> a;
    ASSERT_TRUE(a.empty());
    a = 7;
    ASSERT_FALSE(a.empty());
}

TEST(any, different_type_after_assignment)
{
    any<16> a(7);
    ASSERT_TRUE(a.is_stored_type<int>());
    ASSERT_FALSE(a.is_stored_type<double>());
    a = 3.14;
    ASSERT_FALSE(a.is_stored_type<int>());
    ASSERT_TRUE(a.is_stored_type<double>());
}

TEST(any, get_good_type)
{
    any<16> a(7);
    auto i = a.get<int>();
    ASSERT_EQ(7, i);
}

TEST(any, get_bad_type)
{
    any<16> a(7);
    EXPECT_THROW(a.get<double>(), std::bad_cast);
}

TEST(any, mutable_get)
{
    any<16> a(7);
    a.get<int>() = 6;
    const any<16>& const_ref = a;
    auto i = const_ref.get<int>();
    ASSERT_EQ(6, i);
}

TEST(any, any_to_any_copy_uninitialized)
{
    any<16> a;
    any<16> b(a);

    ASSERT_TRUE(a.empty());
    ASSERT_TRUE(b.empty());
}

TEST(any, any_to_any_copy_construction)
{
    any<16> a(7);
    any<16> b(a);

    ASSERT_EQ(7, a.get<int>());
    ASSERT_EQ(7, b.get<int>());
}

TEST(any, any_to_any_assignment)
{
    any<32> a(std::string("Hello"));
    any<32> b;

    ASSERT_TRUE(b.empty());
    b = a;
    ASSERT_FALSE(b.empty());

    ASSERT_EQ("Hello", b.get<std::string>());
    ASSERT_EQ("Hello", a.get<std::string>());
}

TEST(any, any_to_any_move_construction)
{
    any<32> a(std::string("Hello"));
    any<32> b = std::move(a);

    ASSERT_FALSE(a.empty());
    ASSERT_FALSE(b.empty());

    ASSERT_EQ("Hello", b.get<std::string>());
}

TEST(any, any_to_bigger_any)
{
    any<16> a(1);
    ASSERT_EQ(1, a.get<int>());

    any<32> b(2);
    b = a;

    ASSERT_EQ(1, b.get<int>());
}

TEST(any, any_to_bigger_any_copy)
{
    any<16> a(1);
    ASSERT_EQ(1, a.get<int>());

    any<32> b = a;
    ASSERT_EQ(1, b.get<int>());
}

struct InitCtor
{
    int x = 1;
    int y = 2;
    InitCtor() = default;
    InitCtor(int x, int y) : x(x), y(y) {}
};

TEST(any, emplace_no_params)
{
    any<32> a;
    a.emplace<InitCtor>();

    ASSERT_FALSE(a.empty());
    EXPECT_EQ(1, a.get<InitCtor>().x);
    EXPECT_EQ(2, a.get<InitCtor>().y);
}

TEST(any, emplace_params)
{
    any<32> a;
    a.emplace<InitCtor>(77, 88);

    ASSERT_FALSE(a.empty());
    EXPECT_EQ(77, a.get<InitCtor>().x);
    EXPECT_EQ(88, a.get<InitCtor>().y);
}

TEST(any, destroyed_after_emplace)
{
    CallCounter::reset_counters();
    {
        any<32> a;
        a.emplace<CallCounter>();
    }

    EXPECT_EQ(1, CallCounter::default_constructions);
    EXPECT_EQ(1, CallCounter::destructions);
}

TEST(any, any_cast_pointer_correct_type)
{
    any<16> a(7);
    ASSERT_EQ(7, *any_cast<int>(&a));
}

TEST(any, any_cast_pointer_constness)
{
    any<16> a(7);
    const auto* a2 = &a;

    auto* pv  = any_cast<int>(&a);
    auto* pv2 = any_cast<int>(a2);

    ASSERT_FALSE(std::is_const<std::remove_pointer<decltype(pv)>::type>::value);
    ASSERT_TRUE(std::is_const<std::remove_pointer<decltype(pv2)>::type>::value);

    ASSERT_EQ(7, *pv);
    ASSERT_EQ(7, *pv2);
}

TEST(any, any_cast_pointer_wrong_type)
{
    any<16> a(7);
    ASSERT_EQ(nullptr, any_cast<float>(&a));
}

TEST(any, any_cast_reference_correct_type)
{
    any<16> a(7);
    ASSERT_EQ(7, any_cast<int>(a));
}


TEST(any, any_cast_reference_constness)
{
    any<16> a(7);
    const auto& a2 = a;

    auto& pv = any_cast<int>(a);
    auto& pv2 = any_cast<int>(a2);

    ASSERT_FALSE(std::is_const<std::remove_reference<decltype(pv)>::type>::value);
    ASSERT_TRUE(std::is_const<std::remove_reference<decltype(pv2)>::type>::value);

    ASSERT_EQ(7, pv);
    ASSERT_EQ(7, pv2);
}

TEST(any, any_cast_reference_wrong_type)
{
    any<16> a(7);
    EXPECT_THROW(any_cast<float>(a), bad_any_cast);
}

TEST(any, any_cast_reference_wrong_type_from_to)
{
    any<16> a(7);

    try {
        auto f = any_cast<float>(a);
        FAIL();
        ASSERT_EQ(.1234, f); // to avoid a warning, never reached
    }
    catch(bad_any_cast& ex) {
        ASSERT_EQ(typeid(int), ex.stored_type());
        ASSERT_EQ(typeid(float), ex.target_type());
    }
}

TEST(any, query_type)
{
    any<16> a(7);
    ASSERT_EQ(typeid(int), a.type());

    a = std::string("f00");
    ASSERT_EQ(typeid(std::string), a.type());
}

