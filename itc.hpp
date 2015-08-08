#include <cstdint>
#include <tuple>
#include <utility>
#include <memory>

namespace itc {

    class vector_clock;
    class itc;

    class event;
    class id;

namespace detail {

    template<typename T>
    class intrusive_tree  {
    public:

        intrusive_tree() = default;

        void set_left(std::unique_ptr<T> left) { _left = std::move(left); }
        void set_right(std::unique_ptr<T> right) { _right = std::move(right); }

        bool has_left() { return !!_left; }
        bool has_right() { return !!_right; }
        bool is_leaf() { return !has_left() && !has_right(); }

        T* get_left() { return _left.get(); }
        T* get_right() { return _right.get(); }

    private:
        template <typename, typename>
        friend class lw_intrusive_tree;

        std::unique_ptr<T> _left;
        std::unique_ptr<T> _right;
    };

    // An intrusive tree node that can be either an internal node
    // with two child pointers, or a leaf node holding some data item
    template <typename T, typename ValueType>
    class lw_intrusive_tree {
    public:
        using leaf_node_type = ValueType;
        using this_type = lw_intrusive_tree<T, leaf_node_type>;
        using internal_node_type = intrusive_tree<T>;

        lw_intrusive_tree(): lw_intrusive_tree(leaf_node_type()) {}

        lw_intrusive_tree(leaf_node_type val): _is_leaf{true} {
            new (&_storage) leaf_node_type (std::move(val));
        }

        void set_left(std::unique_ptr<T> left) {
            _ensure_internal_node();
            internal().set_left(std::move(left));
        }

        void set_right(std::unique_ptr<T> right) {
            _ensure_internal_node();
            internal().set_right(std::move(right));
        }

        void set_value(leaf_node_type value) {
            _ensure_leaf_node();
            leaf() = std::move(value);
        }

        bool is_leaf() { return _is_leaf; }

        ~lw_intrusive_tree() {
            if (is_leaf()) {
                _destroy_leaf();
            } else {
                _destroy_internal();
            }
        }

        leaf_node_type& leaf() { return *reinterpret_cast<leaf_node_type*>(&_storage); }
        internal_node_type& internal() { return *reinterpret_cast<internal_node_type*>(&_storage); }

    private:

        void _ensure_internal_node(internal_node_type node = internal_node_type()) {
            if (is_leaf()) {
                _destroy_leaf();
                _is_leaf = false;
                new (&_storage) internal_node_type (std::move(node));
            }
        }

        void _ensure_leaf_node(leaf_node_type node = leaf_node_type()) {
            if (!is_leaf()) {
                _destroy_internal();
                _is_leaf = true;
                new (&_storage) leaf_node_type (std::move(node));
            }
        }

        void _destroy_leaf() {
            leaf().~leaf_node_type();
        }

        void _destroy_internal() {
            internal().~internal_node_type();
        }

        bool _is_leaf;
        typename std::aligned_union<2, leaf_node_type, internal_node_type>::type _storage;
    };

    enum class zero_or_one : std::uint8_t { zero = 0, one = 1 };
}  // namespace detail

    // TODO, fiture out how to use private inheritance without obstructing tests
    class id final : public detail::lw_intrusive_tree<id, detail::zero_or_one> {
        using id_tree = detail::lw_intrusive_tree<id, detail::zero_or_one>;
    public:
        id() = default;
        explicit id(detail::zero_or_one val) : id_tree(val) {}

        static id one();
        static id zero();

        ///
        /// norm (0, 0) = 0
        /// norm (1, 1) = 1
        /// norm (i) = i
        ///
        void normalize() {
            if (is_leaf()) {
                return;
            }

            bool has_left = internal().has_left();
            bool has_right = internal().has_right();

            if (has_left) { internal().get_left()->normalize(); }
            if (has_right) { internal().get_right()->normalize(); }

            if (!has_left || !has_right) {
                return;
            }

            auto left = internal().get_left();
            auto right = internal().get_right();

            if (left->is_leaf() && right->is_leaf() && (left->leaf() == right->leaf())) {
                set_value(std::move(left->leaf()));
            }
        }
    };

    id id::zero() { return id{detail::zero_or_one::zero}; }
    id id::one() { return id{detail::zero_or_one::one}; }

    class event : private detail::intrusive_tree<event> {
    public:
        event() = default;

    private:
        std::uint64_t _value;
    };

    class stamp {
    public:

        // stamp() {
        // }

        // stamp(const stamp& a, const stamp& b) {
        // }

        // std::tuple<stamp, stamp> fork() {

        // }

        // stamp event() {
        // }


    private:
        class event _event;
        id _id;
    };

}  // namespace itc
