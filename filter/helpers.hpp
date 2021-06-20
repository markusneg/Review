
// for arma vector
template <typename T>
inline double vectorLength(const T& vec)
{
    double squareSum = 0;
    for(auto v : vec)
        squareSum += pow(v, 2);

    return sqrt(squareSum);
}
