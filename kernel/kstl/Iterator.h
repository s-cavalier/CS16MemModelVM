#ifndef M__ITERATOR_H__
#define M__ITERATOR_H__

namespace ministl {
    using ptrdiff_t = long;
    struct input_iterator_tag {};
    struct output_iterator_tag {};
    struct forward_iterator_tag       : input_iterator_tag {};
    struct bidirectional_iterator_tag : forward_iterator_tag {};
    struct random_access_iterator_tag : bidirectional_iterator_tag {};
}


#endif