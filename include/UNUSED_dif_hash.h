#ifndef DIF_HASH_H_
#define DIF_HASH_H_

static constexpr long getChrHash(const char c, int index)
{
    index += 7 + 9 + 6 + 5 + 3 + 4 + 2 + 1 + 4 + 1 + 1;
    return (long)c * (long)index * (long)index;
}

constexpr long difGetOprHashBySize(const char* const name, int size)
{
    long hash = 0;
    for (int i = 0; name[i] && i < size; i++)
        hash += getChrHash(name[i], i);

    return hash;
}

constexpr long difGetOprHash(const char* const name)
{
    return difGetOprHashBySize(name, __INT_MAX__);
}

#endif