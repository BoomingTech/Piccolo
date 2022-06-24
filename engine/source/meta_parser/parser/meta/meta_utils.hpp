namespace Utils
{
    template<typename A, typename B>
    bool rangeEqual(A startA, A endA, B startB, B endB)
    {
        while (startA != endA && startB != endB)
        {
            if (*startA != *startB)
                return false;

            ++startA;
            ++startB;
        }

        return (startA == endA) && (startB == endB);
    }
}