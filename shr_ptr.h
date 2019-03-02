//
// Created by roman on 26.01.19.
//


#ifndef EXAM_SECOND_TERM_SHR_PTR_H
#define EXAM_SECOND_TERM_SHR_PTR_H

#include <iostream>

namespace ex {

    struct info_base {
        virtual void destroy() = 0;

        virtual ~info_base() = default;

        long ref_count = 0;
        long weak_ref_count = 0;
    };

    template<typename T>
    struct info : info_base {
        T *get() {
            return ptr;
        }

        void destroy() {
            delete ptr;
            ptr = nullptr;
        }

        info(T *ptr) : ptr(ptr) {}

        ~info() {
            if (ptr)
                destroy();
        }

        T *ptr;
    };

    template<typename T>
    struct weak_ptr;

    template<typename T>
    struct shr_ptr {
        template<typename U>
        friend
        struct shr_ptr;
        template<typename U>
        friend
        struct weak_ptr;
    public:
        shr_ptr() noexcept {
            set_pointers(nullptr, nullptr);
        }

        explicit shr_ptr(T *p) {
            set_pointers(p, nullptr);
            try {
                add_ref();
            } catch (...) {
                delete p;
                throw;
            }
        }

        shr_ptr(std::nullptr_t) noexcept {
            set_pointers(nullptr, nullptr);
        }

        template<typename Y>
        shr_ptr(const shr_ptr<Y> &other) noexcept {
            set_pointers(other.ptr, other.data);
            if (data)
                ++data->ref_count;
        }

        shr_ptr(const shr_ptr &other) noexcept {
            set_pointers(other.ptr, other.data);
            if (data)
                ++data->ref_count;
        }

        template<typename Y>
        shr_ptr &operator=(const shr_ptr<Y> &other) noexcept {
            shr_ptr tmp(other);
            this->swap(tmp);
            return *this;
        }

        shr_ptr &operator=(const shr_ptr &other) noexcept {
            shr_ptr tmp(other);
            this->swap(tmp);
            return *this;
        }

        template<class Y>
        shr_ptr(const shr_ptr<Y> &other, T *p) noexcept {
            set_pointers(p, other.data);
            if (data)
                ++data->ref_count;
        }

        shr_ptr(shr_ptr &&other) noexcept {
            set_pointers(other.ptr, other.data);
            other.set_pointers(nullptr, nullptr);
        }

        shr_ptr &operator=(shr_ptr &&other) noexcept {
            shr_ptr tmp(std::move(other));
            swap(tmp);
            return *this;
        }

        void reset() {
            shr_ptr tmp;
            swap(tmp);
        }

        template<typename Y>
        void reset(Y *p) {
            shr_ptr tmp(p);
            swap(tmp);
        }

        T *get() const {
            return ptr;
        }

        T *operator->() const {
            return get();
        }

        T &operator*() const {
            return *get();
        }

        long use_count() const {
            return data ? data->ref_count : 0;
        }

        bool unique() const {
            return use_count() == 1;
        }

        operator bool() const {
            return get() != nullptr;
        }

        template<class Y>
        explicit shr_ptr(const weak_ptr<Y> &other) {
            set_pointers(other.ptr, other.data);
            if (data)
                ++data->ref_count;
        }

        ~shr_ptr() {
            remove_ref();
        }

        void swap(shr_ptr &other) {
            std::swap(ptr, other.ptr);
            std::swap(data, other.data);
        }

    private:
        void set_pointers(T *p, info_base *d) {
            ptr = p;
            data = d;
        }

        void add_ref() {
            if (!data && ptr)
                data = new info<T>(ptr);
            if (ptr)
                ++data->ref_count;
        }

        void remove_ref() {
            if (!data || --data->ref_count != 0)
                return;
            if (data->weak_ref_count == 0) {
                delete data;
                data = nullptr;
            } else {
                data->destroy();
            }
        }

    private:
        T *ptr;
        info_base *data;
    };


    template<typename T>
    struct weak_ptr {
        template<typename U>
        friend
        struct shr_ptr;
        template<typename U>
        friend
        struct weak_ptr;


        constexpr weak_ptr() noexcept {
            set_pointers(nullptr, nullptr);
        }

        weak_ptr(const weak_ptr &other) noexcept {
            set_pointers(other.ptr, other.data);
            add_weak_ref();
        }

        template<class Y>
        weak_ptr(const shr_ptr<Y> &other) noexcept {
            set_pointers(other.ptr, other.data);
            add_weak_ref();
        }

        weak_ptr &operator=(const weak_ptr &other) noexcept {
            weak_ptr tmp(other);
            swap(tmp);
            return *this;
        }

        template<typename Y>
        weak_ptr &operator=(const shr_ptr<Y> &other) noexcept {
            weak_ptr tmp(other);
            swap(tmp);
            return *this;
        }

        void swap(weak_ptr &other) noexcept{
            std::swap(ptr, other.ptr);
            std::swap(data, other.data);
        }


        shr_ptr<T> lock() const noexcept {
            return data->ref_count == 0? shr_ptr<T>() : shr_ptr<T>(*this);
        }

        ~weak_ptr() {
            remove_weak_ref();
        }

    private:
        void set_pointers(T *p, info_base *d) {
            ptr = p;
            data = d;
        }

        void add_weak_ref() {
            if (data)
                ++data->weak_ref_count;
        }

        void remove_weak_ref() {
            if (!data || --data->weak_ref_count != 0)
                return;
            if (data->ref_count == 0)
                delete data;
            data = nullptr;
        }

    private:
        T *ptr;
        info_base *data;

    };

#endif //EXAM_SECOND_TERM_SHR_PTR_H

}