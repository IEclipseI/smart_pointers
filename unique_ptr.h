//
// Created by roman on 27.01.19.
//

#ifndef EMPTY_FOR_FUN_UNIQUE_PTR_H
#define EMPTY_FOR_FUN_UNIQUE_PTR_H


#include <utility>
#include <stddef.h>
#include <memory>
#include <iostream>

namespace ex {

    struct deleter_base {
        virtual void destroy() = 0;

        virtual ~deleter_base() = default;
    };

    template<typename T, typename Del>
    struct custom_deleter : deleter_base {
        custom_deleter(T *ptr, const Del &del) : ptr(ptr), del(del) {}
        custom_deleter(T *ptr, Del&& del) : ptr(ptr), del(std::move(del)) {}

        void destroy() override {
            del(ptr);
        }

        T *ptr;
        Del del;
    };

    template<typename T>
    struct unique_ptr {
        template<typename U>
        friend struct unique_ptr;

        template<typename U, typename ...Args>
        friend unique_ptr<U> make_unique(Args &&...args);

    public:
        unique_ptr() noexcept : del(nullptr) {}

        unique_ptr(nullptr_t) noexcept : del(nullptr) {}

        unique_ptr(T *ptr) : del(nullptr), ptr(ptr) {}

        unique_ptr(unique_ptr &&other) noexcept: del(other.del), ptr(other.ptr) {
            other.del = nullptr;
            other.ptr = nullptr;
        }

        template<typename Del>
        unique_ptr(T *ptr, const Del &del) : del(new custom_deleter<T, Del>(ptr, del)), ptr(ptr) {}

        template<typename Del>
        unique_ptr(T *ptr, Del&& del) : del(new custom_deleter<T, Del>(ptr, std::forward<Del>(del))), ptr(ptr) {}


        template<typename U, typename = std::enable_if<std::is_convertible<U, T>::value>>
        unique_ptr(unique_ptr<U> &&other) {
            unique_ptr tmp(std::move(other));
            swap(tmp);
        }

        unique_ptr &operator=(unique_ptr &&other) {
            unique_ptr tmp(std::move(other));
            swap(tmp);
            return *this;
        }

        void swap(unique_ptr& other) {
            std::swap(del, other.del);
            std::swap(ptr, other.ptr);
        }

        ~unique_ptr() {
            destructor();
        }

        T *release() {
            T* copy = ptr;
            if (del)
                delete del;
            del = nullptr;
            ptr = nullptr;
            return copy;
        }

        template<typename Y>
        void reset(Y *p) {
            destructor();
            ptr = p;
        }

        template<typename Y, typename Del>
        void reset(Y* p, Del d) {
            destructor();
            del = new custom_deleter<T, Del>(p, d);
            ptr = p;
        }

        T *get() {
            return ptr;
        }

        explicit operator bool() const noexcept{
            return ptr != nullptr;
        }

    private:
        void destructor() {
            if (!del)
                if (!ptr) {
                    return;
                } else {
                    delete ptr;
                    return;
                }
            del->destroy();
            delete del;
        }

    private:
        deleter_base *del;
        T *ptr;
    };

    template<typename T, typename ...Args>
    unique_ptr<T> make_unique(Args &&...args) {
        return unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    template<typename T>
    void swap(unique_ptr<T>& a, unique_ptr<T>& b) {
        a.swap(b);
    }

}

#endif //EMPTY_FOR_FUN_UNIQUE_PTR_H
