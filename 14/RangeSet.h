//
// A RangeSet is a map or set over a large domain, such as size_t,
// where adjacent equivalent elements (by key, not value) are merged.
// 
// This is ideal (seeming?) for something like a memory map or pagetable, where
// typically large contiguous ranges are unallocated, commited, readonly, etc.
// 
// That is not to imply that a multi-level array is not a bad representation.
//
// The underlying structure maps only from start to data.
// The fact that each start has an end is managed here.
//
// Insert dominates/replaces prior insert.
// Insert never fails due to prior data.
//
// Insert and remove implies many split/merge scenarios.
//
// For explanation purposes, we will consider
// ranges that can be blue, red, or empty.
//
// Examples include:
//   prior  bb bb
//   insert  rrr
//   result brrrb
//
//   prior    b
//   insert  rrr
//   result  rrr
//
//   prior   bbb
//   insert   r
//   result  brb
//
//   prior   bb
//   insert   rr
//   result  brr
//
//   prior    bbb
//   insert  rrr
//   result  rrrb
//
//   prior   bbbb
//   insert  rrrr
//   result  rrrr
//
//   prior   bbb
//   remove  x
//   result   bb
//
//   prior   bbb
//   remove   x
//   result  b b
//
//   prior   bbb
//   remove    x
//   result  bb
//
//   prior   bbb
//   remove  xx
//   result    b
//
//   prior   bbb
//   remove   xx
//   result  b
//
//   prior    bbb
//   remove  xxxxx
//   result  empty
//
//   prior  bb 
//   insert    rr
//   result bb rr
//
template <typename Data, typename Int = size_t>
struct RangeSet
{
    struct Entry
    {
        Int start;
        Int end;
        Data data;

        size_t size() const
        {
            return m_end - m_start;
        }

        bool operator==(const Entry& other) const
        {
            return m_start == other.m_start;
        }

        bool operator<(const Entry& other) const
        {
            return m_start < other.m_start;
        }
    };

    std::set<Entry> data;

    void Remove(Int start, Int end)
    {
        // J is the element after end.
        iterator J = erase(lower_bound(start), upper_bound(end));

        if (J == end())
            return;

        // Removal can enable merge of elements previously divided.
        iterator I = J;
        --I;
        if (I->data == J->data)
        {
            I->end += J->end - J->start;
            data.erase(J);
        }
    }

    void Add(Int start, Int end, const Data& data)
    {
#if 0
        Entry e = {start, end};
        pair<iterator, bool> p = this->data.insert(e);

        // Underlying set only understand the start of the range,
        // not size or data, nor splitting or merging.
        //
        // Initially there are very many cases, but some can be combined.

        if (p.second)
        {
            iterator next = p.first;
            ++next;
            bool erased = false;
            bool copied = false;
            if (p.first != begin())
            {
                iterator prev = p.first;
                --prev;
                if (prev->end < start)
                {
                    // Previous is not adjacent or overlapping new, do nothing,
                    // but keep new add.
                    p.first->data = data;
                    copied = true;
                }
                else if (prev->end >= start)
                {
                    if (prev->data == data)
                    {
                        // Previous has same data and adjacent or overlaps.
                        // Extend previous to max of itself and new add,
                        // and erase new add.
                        //
                        // Consider optimizing this case to avoid the insert/erase cycle.
                        prev->end = std::max(end, prev->end);
                        erase(p.first);
                        erased = true;
                    }
                    else
                    {
                        // Previous has different data and is adjacent or overlaps.
                        // Truncate previous to end at new add, and keep new add.
                        prev->end = start;
                        p.first->data = data;
                        copied = true;
                    }
                }
            }

            if (next != this->end())
            {
                if (next->start > end || (next->start == end && next->data != data))
                {
                    // Next is not adjacent, or is adjacent with different data.
                    if (!copied)
                    {
                        p.first->data = data;
                    }
                }
                else
                {
                    if (next->start == end && next->data == data)
                    {
                        // Next is adjacent or overlapping with same data.
                        // Extent next back to new add.
                        // This might further merge with previous.
                    }
                }
            }
        }

        iterator L = lower_bound(start);
        iterator U = upper_bound(end);

        if (L == data.end())
        {
        }
#else
        if (start == end)
            return;

        iterator L = lower_bound(start);

        if (L != data.end())
        {
            if (L->start == start)
            {
                if (L->data == data)
                {
                    // The new add is already covered by existing data.
                    if (L->end >= end)
                    {
                        // For example:
                        // prior  bb
                        // insert b
                        // result bb
                        return;
                    }
                    // Extend existing element to cover new add.
                    // Almost done, but this can also cause merge with arbitrarily
                    // more elements, erasing them and possibly truncating last.
                    L->end = end;
                    iterator U = lower_bound(end);

                    if (U == this->end())
                    {
                        --U;
                        if (L == U)
                            return;
                    }

                    // prior  brrb
                    // insert bb
                    // result bb b
                    //

                    // prior  b rb
                    // insert bb
                    // result bb b
                    //

                    // prior  b  b
                    // insert bb
                    // result bb b
                    //

                    if (U != this->end() && U->start > end)
                    {
                        // For example:
                        // prior  brrb
                        // insert bb
                        // result bb b
                        //
                        // prior  br  b
                        // insert bbb
                        // result bbb b
                        ++L;
                        erase(L, U);
                        ++cov[__LINE__];
                        return;
                    }

                    if (U != this->end() && U->start == end && U->data == data)
                    {
                        L->end = U->end;
                        ++U;
                        ++L;
                        erase(L, U);
                        // For example:
                        // prior  b b
                        // insert bb
                        // result bbb
                        ++cov[__LINE__];
                        return;
                    }
                    if (U != this->end() && U->start == end && U->data != data)
                    {
                        ++L;
                        erase(L, U);
                        // For example:
                        // prior  brbr
                        // insert bbb
                        // result bbbr
                        ++cov[__LINE__];
                        return;
                    }
                    if (U != this->end() && U->start < end && U->data != data)
                    {
                        ++L;
                        U->start = end;
                        erase(L, U);
                        // For example:
                        // prior  brbrrb
                        // insert bbbb
                        // result bbbbrb
                        ++cov[__LINE__];
                        return;
                    }
                    if (U != this->end() && U->start < end && U->data == data)
                    {
                        L->end = U->end;
                        ++L;
                        ++U;
                        erase(L, U);
                        // For example:
                        // prior  brrbbb
                        // insert bbbb
                        // result bbbbbb
                        ++cov[__LINE__];
                        return;
                    }

                    if (U == this->end())
                    {
                        --U;
                        if (L == U)
                        {
                            // For example:
                            // prior  b
                            // insert bb
                            // result bb
                            return;
                        }
                        // For example:
                        // prior  br
                        // insert bb
                        // result bb
                        L->end = U->end;
                        ++U;
                        ++L;
                        erase(L, U);
                        // For example:
                        // prior  brbr
                        // insert bb
                        // result bbbr
                        ++cov[__LINE__];
                        return;
                    }

                    iterator Uprev = U;
                    --Uprev;
                    if (
                    erase(L + 1, U - 1);

                    while (true)
                    {
                        iterator next = L;
                        ++next;
                    }
                }
                else
                {
                }
            }
        }
#endif
    }
};
