# static_vector

A stack based alternative to std::vector with statically deffined capacity.
It implements the same API as std::vector with the notable exceptions:
* It has no notion of allocators.
* When attempting to insert more elements then the capacity allows, it will throw an std::out_of_range exception instead of reallocating.
* If any of move/copy constructors, assignment operators or swap() may fail because insufficient capacity in one of the static_vectors, no exception will be thrown and the operation will go through, but the overflowing elements will be destroyed.
