//===- InstrumentationMap.h - XRay Instrumentation Map --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Defines the interface for extracting the instrumentation map from an
// XRay-instrumented binary.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_XRAY_INSTRUMENTATION_MAP_H
#define LLVM_XRAY_INSTRUMENTATION_MAP_H

#include <vector>
#include <string>
#include <unordered_map>

#include "llvm/ADT/Optional.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/YAMLTraits.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {
namespace xray {

// Forward declare to make a friend.
class InstrumentationMap;

/// Loads the instrumentation map from |Filename|. This auto-deduces the type of
/// the instrumentation map.
Expected<InstrumentationMap> loadInstrumentationMap(StringRef Filename);

/// Represents an XRay instrumentation sled entry from an object file.
struct SledEntry {

  /// Each entry here represents the kinds of supported instrumentation map
  /// entries.
  enum class FunctionKinds { ENTRY, EXIT, TAIL };

  /// The address of the sled.
  uint64_t Address;

  /// The address of the function.
  uint64_t Function;

  /// The kind of sled.
  FunctionKinds Kind;

  /// Whether the sled was annotated to always be instrumented.
  bool AlwaysInstrument;
};

struct YAMLXRaySledEntry {
  int32_t FuncId;
  yaml::Hex64 Address;
  yaml::Hex64 Function;
  SledEntry::FunctionKinds Kind;
  bool AlwaysInstrument;
};

/// The InstrumentationMap represents the computed function id's and indicated
/// function addresses from an object file (or a YAML file). This provides an
/// interface to just the mapping between the function id, and the function
/// address.
///
/// We also provide raw access to the actual instrumentation map entries we find
/// associated with a particular object file.
///
class InstrumentationMap {
public:
  using FunctionAddressMap = std::unordered_map<int32_t, uint64_t>;
  using FunctionAddressReverseMap = std::unordered_map<uint64_t, int32_t>;
  using SledContainer = std::vector<SledEntry>;

private:
  SledContainer Sleds;
  FunctionAddressMap FunctionAddresses;
  FunctionAddressReverseMap FunctionIds;

  friend Expected<InstrumentationMap> loadInstrumentationMap(StringRef);

public:
  /// Provides a raw accessor to the unordered map of function addresses.
  const FunctionAddressMap &getFunctionAddresses() { return FunctionAddresses; }

  /// Returns an XRay computed function id, provided a function address.
  Optional<int32_t> getFunctionId(uint64_t Addr) const;

  /// Returns the function address for a function id.
  Optional<uint64_t> getFunctionAddr(int32_t FuncId) const;

  /// Provide read-only access to the entries of the instrumentation map.
  const SledContainer &sleds() const { return Sleds; };
};

} // namespace xray
} // namespace llvm

namespace llvm {
namespace yaml {

template <> struct ScalarEnumerationTraits<xray::SledEntry::FunctionKinds> {
  static void enumeration(IO &IO, xray::SledEntry::FunctionKinds &Kind) {
    IO.enumCase(Kind, "function-enter", xray::SledEntry::FunctionKinds::ENTRY);
    IO.enumCase(Kind, "function-exit", xray::SledEntry::FunctionKinds::EXIT);
    IO.enumCase(Kind, "tail-exit", xray::SledEntry::FunctionKinds::TAIL);
  }
};

template <> struct MappingTraits<xray::YAMLXRaySledEntry> {
  static void mapping(IO &IO, xray::YAMLXRaySledEntry &Entry) {
    IO.mapRequired("id", Entry.FuncId);
    IO.mapRequired("address", Entry.Address);
    IO.mapRequired("function", Entry.Function);
    IO.mapRequired("kind", Entry.Kind);
    IO.mapRequired("always-instrument", Entry.AlwaysInstrument);
  }

  static constexpr bool flow = true;
};
} // namespace yaml
} // namespace llvm

LLVM_YAML_IS_SEQUENCE_VECTOR(xray::YAMLXRaySledEntry)

#endif // LLVM_XRAY_INSTRUMENTATION_MAP_H
