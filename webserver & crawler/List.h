//
// Created by spyros on 28/2/2018.
//
#ifndef SEARCH_ENGINE_STRINGMAP_H
#define SEARCH_ENGINE_STRINGMAP_H
template <class Items>
class List {
public:
    List(int initial_size);
    List(int initial_size, int noDeleteNodes);
    ~List();
    Items &operator[](int i) const;
    Items value(int i) const;
    Items remove();
    void   update(int i, Items item);
    int getLen() const;
    void appendElement(Items item);
    bool exists(Items query);
    int start_from;
    Items *Array;
private:
    int len;
    int allocated;
    void allocate_more();
    int noDeleteNodes;
};

#endif //SEARCH_ENGINE_STRINGMAP_H
