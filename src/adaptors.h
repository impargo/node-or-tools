#ifndef NODE_OR_TOOLS_ADAPTORS_F1FF74E9681C_H
#define NODE_OR_TOOLS_ADAPTORS_F1FF74E9681C_H

#include <nan.h>

#include "types.h"

// We adapt matrices and vectors for or-tools since it expects them to have specific signatures and types.

// Matrix to operator()(NodeIndex, NodeIndex);
template <typename T> auto makeBinaryAdaptor(const T& m) {
  return [&](NodeIndex from, NodeIndex to) -> int64 { return m.at(from.value(), to.value()); };
}

// Vector to operator()(NodeIndex);
template <typename T> auto makeUnaryAdaptor(const T& v) {
  return [&](NodeIndex idx) -> int64 { return v.at(idx.value()); };
}

// Adaptors to callback. Note: ownership is bound to the underlying storage.
template <typename Adaptor> auto makeCallback(const Adaptor& adaptor) {
  return NewPermanentCallback(&adaptor, &Adaptor::operator());
}

// Caches user provided Function(s, t) -> Number into Matrix
template <typename Matrix> inline auto makeMatrixFromFunction(std::int32_t n, v8::Local<v8::Function> fn) {
  if (n < 0)
    throw std::runtime_error{"Negative dimension"};

  Nan::Callback callback{fn};

  Matrix matrix{n};

  for (std::int32_t fromIdx = 0; fromIdx < n; ++fromIdx) {
    for (std::int32_t toIdx = 0; toIdx < n; ++toIdx) {
      const auto argc = 2u;
      v8::Local<v8::Value> argv[argc] = {Nan::New(fromIdx), Nan::New(toIdx)};

      auto cost = callback.Call(argc, argv);

      if (!cost->IsNumber())
        throw std::runtime_error{"Expected function signature: Number fn(Number from, Number to)"};

      matrix.at(fromIdx, toIdx) = Nan::To<std::int32_t>(cost).FromJust();
    }
  }

  return matrix;
}

// Caches user provided Js Array into a Vector
template <typename Vector> inline auto makeVectorFromJsNumberArray(v8::Local<v8::Array> array) {
  const std::int32_t len = array->Length();

  Vector vec(len);

  for (std::int32_t atIdx = 0; atIdx < len; ++atIdx) {
    auto num = Nan::Get(array, atIdx).ToLocalChecked();

    if (!num->IsNumber())
      throw std::runtime_error{"Expected array element of types Number"};

    vec.at(atIdx) = Nan::To<std::int32_t>(num).FromJust();
  }

  return vec;
}

#endif
