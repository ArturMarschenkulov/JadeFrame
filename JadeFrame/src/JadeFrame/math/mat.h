#pragma once
#include "JadeFrame/prelude.h"
#include "vec.h"
#include <cmath>
#include <concepts>
#include <type_traits>

/*
        This matrix is column major

        mat[n][m]
        mat[col][row]
        n = columns
        m = rows

        n0 n1 n2 (rank)
         __ __ __
m0	|__|__|__|
m1	|__|__|__|
m2	|__|__|__|

*/
namespace JadeFrame {
template<size_t N, size_t M, typename T>
// requires number<T>
class MatrixT {
public:
    constexpr MatrixT() noexcept
        : el() {}

    constexpr MatrixT(const T digo)
        : el() {
        static_assert(N == M);
        for (u32 i = 0; i < N; i++) { el[N][M] = digo; }
    }

    template<N1, M1>
    constexpr auto operator*(const MatrixT<N1, M1>& other) const noexcept -> MatrixT<N1, M> {
        MatrixT<N1, M> result;

        return result;
    }

    template<N1>
    constexpr auto operator*(const VectorT<N1>& other) const noexcept -> VectorT<N1> {
        VectorT<N1> result;

        return result;
    }

public:
    std::array<std::array<T, N>, M> el;
};

template<typename T>
class MatrixT<4, 4, T> {
    constexpr MatrixT() {}

private:
    union {
        std::array<std::array<T, N>, M> el;
        // std::array<f32, 4> colVec;
    };
};

using m2x2f32 = MatrixT<2, 2, f32>;
using m3x3f32 = MatrixT<3, 3, f32>;
using m4x4f32 = MatrixT<4, 4, f32>;

using m2x2 = m2x2f32;
using m3x3 = m3x3f32;
using m4x4 = m4x4f32;

using m2 = m2x2;
using m3 = m3x3;
using m4 = m4x4;


} // namespace JadeFrame
