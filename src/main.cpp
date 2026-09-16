#include "SharedPtr.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>

int main() {
    // constructor + operator*
    SharedPtr<int> ptr(new int(1));
    assert(*ptr == 1);

    // get()
    SharedPtr<int> ptr2(new int(2));
    assert(ptr2.get() != nullptr);

    // operator bool on an empty pointer
    SharedPtr<int> empty;
    assert(!empty);

    // useCount
    assert(ptr.useCount() == 1 && empty.useCount() == 0);

    // operator->
    SharedPtr<std::string> ptrStr(new std::string("hi"));
    assert(ptrStr->size() == 2);

    // copy constructor: both share the object, refcount goes to 2
    SharedPtr<int> copy1(ptr);
    assert(*copy1 == 1 && copy1 == ptr && ptr.useCount() == 2);

    // copy assignment: old value (99) is released, copy2 joins ptr's group
    SharedPtr<int> copy2(new int(99));
    copy2 = ptr;
    assert(*copy2 == 1 && ptr.useCount() == 3);

    // destructor: one owner going out of scope decrements the count
    {
        SharedPtr<int> scoped(ptr);
        assert(ptr.useCount() == 4);
    }
    assert(ptr.useCount() == 3);

    // move constructor: source is emptied, count unchanged
    SharedPtr<int> ptr3 = std::move(copy1);
    assert(!copy1 && *ptr3 == 1 && ptr3.useCount() == 3);

    // move assignment: old value (4) is released, source is emptied, count unchanged
    SharedPtr<int> ptr4(new int(4));
    ptr4 = std::move(copy2);
    assert(!copy2 && *ptr4 == 1 && ptr.useCount() == 3);

    // operator== and operator!=
    SharedPtr<int> other(new int(1));
    assert(ptr == ptr3);  // same object
    assert(ptr != other); // different objects, even with equal values
    assert(ptr != empty);

    // swap: pointers exchange, refcounts move with control block
    SharedPtr<int> ptr11(new int(1));
    SharedPtr<int> ptr12(new int(2));
    SharedPtr<int> ptr12b(ptr12); // give ptr12's group a second owner
    ptr11.swap(ptr12);
    assert(*ptr11 == 2 && *ptr12 == 1);
    assert(ptr11.useCount() == 2 && ptr12.useCount() == 1);

    // reset(): emptied, the other owner is unaffected
    SharedPtr<int> ptr13(new int(13));
    SharedPtr<int> ptr13b(ptr13);
    ptr13.reset();
    assert(!ptr13 && ptr13.useCount() == 0);
    assert(*ptr13b == 13 && ptr13b.useCount() == 1);

    // reset(p): old value (14) is released, 15 is now managed
    SharedPtr<int> ptr14(new int(14));
    ptr14.reset(new int(15));
    assert(*ptr14 == 15 && ptr14.useCount() == 1);

    // reset(get()): self-reset does nothing, no accidental deletion
    SharedPtr<int> ptr15(new int(16));
    ptr15.reset(ptr15.get());
    assert(*ptr15 == 16 && ptr15.useCount() == 1);

    // makeSharedBasic
    SharedPtr<int> made = makeSharedBasic<int>(10);
    assert(*made == 10 && made.useCount() == 1);

    // aliasing constructor
    struct Pair { int first; int second; };
    SharedPtr<Pair> pairPtr = makeSharedBasic<Pair>(Pair{1, 2});
    SharedPtr<int> secondPtr(pairPtr, &pairPtr->second);
    assert(*secondPtr == 2 && pairPtr.useCount() == 2);
    pairPtr.reset(); // the Pair stays alive, secondPtr still owns it
    assert(*secondPtr == 2 && secondPtr.useCount() == 1);

    std::cout << "PASSED" << std::endl;
    return EXIT_SUCCESS;
}
