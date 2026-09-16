#ifndef SHARED_PTR_HEADER
#define SHARED_PTR_HEADER
#include <cassert> // assert
#include <utility> // std::forward
class ControlBlockBase {
private:
    // refcount field
    long counter;
public:
    // constructor
    ControlBlockBase() : counter(1) {}

    // dtor is virtual, so that we can call derived class's dtor from a ptr to this base class.
    virtual ~ControlBlockBase(){}

    // pure virtual function; must be overriden by derived classes
    virtual void* managedAddress() = 0;

    // Delete copies, which also implicitly deletes moves.
    ControlBlockBase(const ControlBlockBase&) = delete;
    ControlBlockBase& operator=(const ControlBlockBase&) = delete;

    long increment()
    {
        return ++counter;
    }

    long decrement()
    {
        assert(counter > 0);
        return --counter;
    }

    long refCount() const
    {
        return counter;
    }
};


template<class T>
class ControlBlock;


template<class T>
class SharedPtr{
private:
    T* ptr;
    ControlBlockBase* controlBlock;
public:
    // no args constructor
    SharedPtr() : ptr(nullptr), controlBlock(nullptr) {}

    // constructor with a pointer argument
    SharedPtr(T* p) : ptr(p), controlBlock(new ControlBlock<T>(p)) {}

    // let SharedPtr<U> access SharedPtr<T>'s private members
    template<class U> friend class SharedPtr;
    // aliasing constructor
    // same control block but manages a different pointer
    // assisted by claude (details in submission text)
    template<class U>
    SharedPtr(const SharedPtr<U>& other, T* p) : ptr(p), controlBlock(other.controlBlock) {
        if (controlBlock != nullptr) {
            controlBlock->increment();
        }
    }

    // destructor
    ~SharedPtr() {
        if (controlBlock != nullptr) {
            if(controlBlock->decrement() == 0) {
                delete controlBlock;
            }
        }
    }

    // copy constructor
    SharedPtr(const SharedPtr& other) : ptr(other.ptr), controlBlock(other.controlBlock) {
        if (controlBlock != nullptr) {
            controlBlock->increment();
        }
    }
    
    // copy assignment
    SharedPtr& operator=(const SharedPtr& other) {
        // increment other
        if (other.controlBlock != nullptr) {
            other.controlBlock->increment();
        }
        // decrement this
        if (controlBlock != nullptr){
            controlBlock->decrement();
            if (controlBlock->refCount() == 0) {
            delete controlBlock;
            }
        }

        // perform copy
        ptr = other.ptr;
        controlBlock = other.controlBlock;

        return *this;
    }

    // move constructor
    SharedPtr(SharedPtr&& other){
            ptr = other.ptr;
            controlBlock = other.controlBlock;
            other.ptr = nullptr;
            other.controlBlock = nullptr;
    }

    // move assignment
    SharedPtr& operator=(SharedPtr&& other){
        if (this != &other){
            if (controlBlock != nullptr){
                controlBlock->decrement();
                if (controlBlock->refCount() == 0) {
                delete controlBlock;
                }
            }

            // perform move
            ptr = other.ptr;
            controlBlock = other.controlBlock;
            other.ptr = nullptr;
            other.controlBlock = nullptr;
        }
        return *this;
    }

    // dereference operator
    T& operator*() const {
        assert(ptr != nullptr);
        return *ptr;
    }

    // arrow operator
    T* operator->() const{
        assert(ptr != nullptr);
        return ptr;
    }

    // get
    T* get() const {
        return ptr;
    }

    // equality operators
    bool operator==(const SharedPtr& other) const {
        return ptr == other.ptr;
    }
    bool operator!=(const SharedPtr& other) const {
        return ptr != other.ptr;
    }

    // bool conversion operator
    operator bool() const {
        return ptr != nullptr;
    }

    // swap
    void swap(SharedPtr& other) {
        // swap pointers
        T* tempptr = ptr;
        ptr = other.ptr;
        other.ptr = tempptr;

        // swap control blocks
        ControlBlockBase* tempcontrolBlock = controlBlock;
        controlBlock = other.controlBlock;
        other.controlBlock = tempcontrolBlock;

        // do not touch refcounts, it comes in the block
    }

    // reset
    void reset(){
        // releases ownership of current object
        SharedPtr().swap(*this); // SharedPtr is a temporary object which will be destroyed
    }

    // reset with new pointer
    void reset(T* p){
        // self-reset: already managing p, do nothing
        if (p == ptr) {
            return;
        }
        // releases ownership of the current object
        // and starts owning/managing p
        SharedPtr(p).swap(*this);
    }

    // use count
    long useCount() const {
        if (controlBlock != nullptr) {
            return controlBlock->refCount();
        }
        return 0;
    }

};

// makeSharedBasic
template<class T, class... Args>
SharedPtr<T> makeSharedBasic(Args&&... args) {
    return SharedPtr<T>(new T(std::forward<Args>(args)...));
}

// control block
template<class T>
class ControlBlock : public ControlBlockBase {
    T* resource;
public:
    // constructor
    ControlBlock(T* res) : resource(res) {}

    // destructor
    ~ControlBlock() {
        delete resource;
    }

    // managedAddress override
    void* managedAddress() override {
        return resource;
    }
};

#endif
