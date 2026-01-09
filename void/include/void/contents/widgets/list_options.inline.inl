#include "list_options.h"


void_begin_

template <typename T>
class list_options_member_constant : public list_options {
private:
    const T* const data_;
    const std::size_t size_;
    xstr T::* member_;

public:
    list_options_member_constant(const T* data, std::size_t size, xstr T::* member)
        : data_(data),
          size_(size),
          member_(member) { }

public:
    virtual bool is_dynamic() const noexcept {
        return false;
    }

    std::size_t size() const noexcept override {
        return size_;
    }

    const xstr& element(std::size_t index) const override {
        return data_[index].*member_;
    }

    const xstr& element_safe(std::size_t index) const override {
        if (index < 0 || index >= size_) {
            return kNone;
        }
        return data_[index].*member_;
    }
};

template <typename T>
class list_options_member_dynamic : public list_options {
private:
    const T** data_;
    std::size_t* size_;
    xstr T::* member_;

public:
    list_options_member_dynamic(T** data, std::size_t* size, xstr T::* member)
        : data_(data),
          size_(size),
          member_(member) { }

public:
    virtual bool is_dynamic() const noexcept {
        return true;
    }

    std::size_t size() const noexcept override {
        return *size_;
    }

    const xstr& element(std::size_t index) const override {
        auto* data = *data_;
        v_assert(data != nullptr);
        v_assert(index >= 0 && index < size());

        return data[index].*member_;
    }

    const xstr& element_safe(std::size_t index) const override {
        auto* data = *data_;
        if (data == nullptr) {
            return kNone;
        }
        if (index < 0 || index >= size_) {
            return kNone;
        }
        return data[index].*member_;
    }
};

template <typename T>
class list_options_vector_member_constant : public list_options {
private:
    const std::vector<T>& data_;
    xstr T::* member_;

public:
    list_options_vector_member_constant(const std::vector<T>& data, xstr T::* member)
        : data_(data),
          member_(member) { }

public:
    virtual bool is_dynamic() const noexcept {
        return false;
    }

    std::size_t size() const noexcept override {
        return static_cast<std::size_t>(data_.size());
    }

    const xstr& element(std::size_t index) const override {
        v_assert(index >= 0 && index < size());
        return data_[static_cast<std::size_t>(index)].*member_;
    }

    const xstr& element_safe(std::size_t index) const override {
        if (index < 0 || index >= size()) {
            return kNone;
        }
        return data_[static_cast<std::size_t>(index)].*member_;
    }
};

template <typename T>
class list_options_vector_member_dynamic : public list_options {
private:
    const std::vector<T>* data_;
    xstr T::* member_;

public:
    list_options_vector_member_dynamic(const std::vector<T>* data, xstr T::* member)
        : data_(data),
          member_(member) { }

public:
    virtual bool is_dynamic() const noexcept {
        return true;
    }

    std::size_t size() const noexcept override {
        return static_cast<std::size_t>(data_->size());
    }

    const xstr& element(std::size_t index) const override {
        v_assert(index >= 0 && index < size());
        return (*data_)[static_cast<std::size_t>(index)].*member_;
    }

    const xstr& element_safe(std::size_t index) const override {
        if (index < 0 || index >= size()) {
            return kNone;
        }
        return (*data_)[static_cast<std::size_t>(index)].*member_;
    }
};

class list_options_constant : public list_options {
private:
    const xstr* const data_;
    const std::size_t size_;

public:
    list_options_constant(const xstr* data, std::size_t size)
        : data_(data), 
          size_(size) { }

public:
    virtual bool is_dynamic() const noexcept {
        return false;
    }

    virtual std::size_t size() const noexcept override {
        return size_;
    }

    virtual const xstr& element(std::size_t index) const override {
        assert(index < size());
        return data_[index];
    }
    virtual const xstr& element_safe(std::size_t index) const override {
        if (index >= size_)
            return kNone;

        return data_[index];
    }
};

class list_options_vector_dynamic : public list_options {
private:
    const std::vector<xstr>* const data_;

public:
    list_options_vector_dynamic(const std::vector<xstr>* data)
        : data_(data) { }

public:
    virtual bool is_dynamic() const noexcept {
        return true;
    }

    virtual std::size_t size() const noexcept override {
        return data_->size();
    }

    virtual const xstr& element(std::size_t index) const override {
        assert(index < size());
        return (*data_)[index];
    }
    virtual const xstr& element_safe(std::size_t index) const override {
        if (index >= data_->size())
            return kNone;

        return (*data_)[index];
    }
};

// 

template<typename T, std::integral I>
inline list_options* list_options::create_member_constant(const T* data, I size, xstr T::* member)
{
    return new list_options_member_constant<T>(
        data, 
        static_cast<std::size_t>(size),
        member
    );
}

template<typename T>
inline list_options* list_options::create_member_dynamic(T** data, std::size_t* size, xstr T::* member)
{
    return new list_options_member_dynamic<T>(
        data,
        static_cast<std::size_t>(size),
        member
    );
}

template<std::integral I>
inline list_options* list_options::create_constant(const xstr* data, I size)
{
    return new list_options_constant(
        data,
        static_cast<std::size_t>(size)
    );
}

template<std::size_t N>
inline list_options* list_options::create_constant(
    const xstr(&data)[N])
{
    return create_constant(&data[0], N);
}

inline list_options* list_options::create_vector_dynamic(const std::vector<xstr>* data)
{
    return new list_options_vector_dynamic(data);
}

template <typename T>
list_options* list_options::create_vector_member_constant(
    const std::vector<T>& data, xstr T::* member)
{
    return new list_options_vector_member_constant<T>(data, member);
}

template <typename T>
list_options* list_options::create_vector_member_dynamic(
    const std::vector<T>* data, xstr T::* member)
{
    return new list_options_vector_member_dynamic<T>(data, member);
}

void_end_