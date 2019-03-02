//
// Created by roman on 19.01.19.
//

#ifndef EXAM_SECOND_TERM_FUNC_H
#define EXAM_SECOND_TERM_FUNC_H

#include <memory>
#include <iostream>

template<typename>
class func;

template<typename Ret, typename ...Args>
struct func<Ret(Args...)> {
    struct concept {
        virtual Ret invoke(Args... args) = 0;

        virtual std::unique_ptr<concept> copy() const = 0;

        virtual void placement_build_small_copy(void *adr) const = 0;

        virtual void placement_build_big_copy(void *adr) const = 0;

        virtual void placement_build_small_move(void *adr) = 0;

        virtual ~concept() = default;
    };

    template<typename T>
    struct model : concept {
        model(const T &t) : t(t) {}

        model(T &&t) : t(std::move(t)) {};

        ~model() = default;

        Ret invoke(Args ...args) override {
            return t(std::forward<Args>(args)...);
        }

        std::unique_ptr<concept> copy() const override {
            return std::make_unique<model<T>>(t);
        }

        void placement_build_small_copy(void *adr) const override {
            new(adr) model<T>(t);
        }

        void placement_build_big_copy(void *adr) const override {
            new(adr) std::unique_ptr<model<T>>(new model<T>(t));
        }

        void placement_build_small_move(void *adr) override {
            new(adr) model<T>(std::move(t));
        }

        T t;
    };

    template<typename T>
    func(T t) {
        std::cout << "constructor(T)\n";
        if constexpr (noexcept(T(std::move(t))) && sizeof(T) <= B_SIZE) {
            is_small = true;
            new(buf) model<T>(std::move(t));
        } else {
            new(buf) std::unique_ptr<concept>(std::make_unique<model<T>>(std::move(t)));
            is_small = false;
        }
    }


    Ret operator()(Args...args)  {
        if (is_small) {
            auto *m = reinterpret_cast<concept*>(buf);
            return m->invoke(std::forward<Args>(args)...);
        }
        return pconcept->invoke(std::forward<Args>(args)...);
    }

    func() noexcept : pconcept(nullptr), is_small(false) {}

    func(std::nullptr_t) noexcept : pconcept(nullptr), is_small(false) {}

    func(const func &other) {
        std::cout << "copy constructor\n";
        auto *c = reinterpret_cast<const concept *>(other.buf);
        if (other.is_small) {
            c->placement_build_small_copy(buf);
        } else {
            c->placement_build_big_copy(buf);
        }
        is_small = other.is_small;
    }

    func(func &&other) noexcept {
        std::cout << "move constructor\n";
        auto c = reinterpret_cast<concept *>(other.buf);
        if (other.is_small) {
            c->placement_build_small_move(buf);
            new(other.buf) std::unique_ptr<concept>(nullptr);
        } else {
            new(buf) std::unique_ptr<concept>(std::move(other.pconcept));
        }
        is_small = other.is_small;
        other.is_small = false;
    }

    func &operator=(const func &other) {
        std::cout << "operator=(const func&)\n";
        func tmp(other);
        swap(tmp);
        return *this;
    }

    func &operator=(func &&other) noexcept {
        std::cout << "operator=(func&&)\n";
        if (is_small) {
            auto c = reinterpret_cast<concept *>(buf);
            c->~concept();
        } else {
            pconcept.~unique_ptr();
        }
        if (other.is_small) {
            auto c = reinterpret_cast<concept *>(other.buf);
            c->placement_build_small_move(buf);
            new(other.buf) std::unique_ptr<concept>(nullptr);
        } else {
            new(buf) std::unique_ptr<concept>(std::move(other.pconcept));
        }
        is_small = other.is_small;
        other.is_small = false;

        return *this;
    }

    void swap(func &other) noexcept {
        func tmp(std::move(other));
        other = std::move(*this);
        *this = std::move(tmp);
    }

    explicit operator bool() const noexcept {
        return is_small || static_cast<bool>(pconcept);
    }

    ~func() {
        if (is_small) {
            auto m = reinterpret_cast<concept *>(buf);
            m->~concept();
        } else {
            pconcept.~unique_ptr();
        }
    }

private:
    static const size_t B_SIZE = 64;
    union {
        std::unique_ptr<concept> pconcept;
        char buf[B_SIZE];
    };
    bool is_small;
};

#endif //EXAM_SECOND_TERM_FUNC_H