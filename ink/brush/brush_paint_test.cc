// Copyright 2024 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ink/brush/brush_paint.h"

#include <limits>
#include <optional>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/hash/hash_testing.h"
#include "absl/log/absl_check.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "ink/geometry/angle.h"
#include "ink/geometry/vec.h"
#include "ink/types/uri.h"

namespace ink {
namespace {

using ::testing::HasSubstr;

static_assert(std::numeric_limits<float>::has_quiet_NaN);
constexpr float kNan = std::numeric_limits<float>::quiet_NaN();
constexpr float kInfinity = std::numeric_limits<float>::infinity();

Uri CreateTestTextureUri() {
  auto uri = Uri::Parse("ink://ink/texture:test-texture");
  ABSL_CHECK_OK(uri);
  return *uri;
}

TEST(BrushPaintTest, TextureKeyframeSupportsAbslHash) {
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      BrushPaint::TextureKeyframe{.progress = 0},
      BrushPaint::TextureKeyframe{.progress = 1},
      BrushPaint::TextureKeyframe{.progress = 0, .size = Vec{1, 1}},
      BrushPaint::TextureKeyframe{.progress = 0, .offset = Vec{1, 1}},
      BrushPaint::TextureKeyframe{.progress = 0, .rotation = kPi},
      BrushPaint::TextureKeyframe{.progress = 0, .opacity = 0.5},
  }));
}

TEST(BrushPaintTest, TextureLayerSupportsAbslHash) {
  absl::StatusOr<Uri> uri1 = Uri::Parse("/texture:foo");
  ASSERT_EQ(uri1.status(), absl::OkStatus());
  absl::StatusOr<Uri> uri2 = Uri::Parse("/texture:bar");
  ASSERT_EQ(uri2.status(), absl::OkStatus());
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      BrushPaint::TextureLayer{.color_texture_uri = *uri1},
      BrushPaint::TextureLayer{.color_texture_uri = *uri2},
      BrushPaint::TextureLayer{.color_texture_uri = *uri1,
                               .mapping = BrushPaint::TextureMapping::kWinding},
      BrushPaint::TextureLayer{
          .color_texture_uri = *uri1,
          .origin = BrushPaint::TextureOrigin::kFirstStrokeInput},
      BrushPaint::TextureLayer{
          .color_texture_uri = *uri1,
          .size_unit = BrushPaint::TextureSizeUnit::kStrokeSize},
      BrushPaint::TextureLayer{.color_texture_uri = *uri1, .size = {2, 2}},
      BrushPaint::TextureLayer{.color_texture_uri = *uri1, .offset = {1, 1}},
      BrushPaint::TextureLayer{.color_texture_uri = *uri1, .rotation = kPi},
      BrushPaint::TextureLayer{.color_texture_uri = *uri1,
                               .size_jitter = {2, 2}},
      BrushPaint::TextureLayer{.color_texture_uri = *uri1,
                               .offset_jitter = {1, 1}},
      BrushPaint::TextureLayer{.color_texture_uri = *uri1,
                               .rotation_jitter = kPi},
      BrushPaint::TextureLayer{.color_texture_uri = *uri1, .opacity = 0.5},
      BrushPaint::TextureLayer{.color_texture_uri = *uri1,
                               .keyframes = {{.progress = 1}}},
      BrushPaint::TextureLayer{.color_texture_uri = *uri1,
                               .blend_mode = BrushPaint::BlendMode::kXor},
  }));
}

TEST(BrushPaintTest, BrushPaintSupportsAbslHash) {
  absl::StatusOr<Uri> uri1 = Uri::Parse("/texture:foo");
  ASSERT_EQ(uri1.status(), absl::OkStatus());
  absl::StatusOr<Uri> uri2 = Uri::Parse("/texture:bar");
  ASSERT_EQ(uri2.status(), absl::OkStatus());
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      BrushPaint{},
      BrushPaint{{{.color_texture_uri = *uri1}}},
      BrushPaint{{{.color_texture_uri = *uri1}, {.color_texture_uri = *uri2}}},
  }));
}

TEST(BrushPaintTest, TextureKeyframeEqualAndNotEqual) {
  BrushPaint::TextureKeyframe keyframe = {
      .progress = 1,
      .size = Vec{2, 2},
      .offset = Vec{1, 1},
      .rotation = kPi,
      .opacity = 0.5,
  };

  BrushPaint::TextureKeyframe other = keyframe;
  EXPECT_EQ(keyframe, other);

  other = keyframe;
  other.progress = 0;
  EXPECT_NE(keyframe, other);

  other = keyframe;
  other.size = Vec{7, 4};
  EXPECT_NE(keyframe, other);

  other = keyframe;
  other.offset = Vec{1, -1};
  EXPECT_NE(keyframe, other);

  other = keyframe;
  other.rotation = std::nullopt;
  EXPECT_NE(keyframe, other);

  other = keyframe;
  other.opacity = 0.25;
  EXPECT_NE(keyframe, other);
}

TEST(BrushPaintTest, TextureLayerEqualAndNotEqual) {
  absl::StatusOr<Uri> uri1 = Uri::Parse("/texture:foo");
  ASSERT_EQ(uri1.status(), absl::OkStatus());
  absl::StatusOr<Uri> uri2 = Uri::Parse("/texture:bar");
  ASSERT_EQ(uri2.status(), absl::OkStatus());
  BrushPaint::TextureLayer layer = {
      .color_texture_uri = *uri1,
      .mapping = BrushPaint::TextureMapping::kTiling,
      .origin = BrushPaint::TextureOrigin::kStrokeSpaceOrigin,
      .size_unit = BrushPaint::TextureSizeUnit::kStrokeCoordinates,
      .size = {1, 1},
      .offset = {0, 0},
      .rotation = Angle(),
      .size_jitter = {0, 0},
      .offset_jitter = {0, 0},
      .rotation_jitter = Angle(),
      .opacity = 1,
      .keyframes = {},
      .blend_mode = BrushPaint::BlendMode::kModulate,
  };

  BrushPaint::TextureLayer other = layer;
  EXPECT_EQ(layer, other);

  other = layer;
  other.color_texture_uri = *uri2;
  EXPECT_NE(layer, other);

  other = layer;
  other.mapping = BrushPaint::TextureMapping::kWinding;
  EXPECT_NE(layer, other);

  other = layer;
  other.origin = BrushPaint::TextureOrigin::kFirstStrokeInput;
  EXPECT_NE(layer, other);

  other = layer;
  other.size_unit = BrushPaint::TextureSizeUnit::kBrushSize;
  EXPECT_NE(layer, other);

  other = layer;
  other.size = {4, 5};
  EXPECT_NE(layer, other);

  other = layer;
  other.offset = {1, -1};
  EXPECT_NE(layer, other);

  other = layer;
  other.rotation = kPi;
  EXPECT_NE(layer, other);

  other = layer;
  other.size_jitter = {4, 5};
  EXPECT_NE(layer, other);

  other = layer;
  other.offset_jitter = {1, -1};
  EXPECT_NE(layer, other);

  other = layer;
  other.rotation_jitter = kPi;
  EXPECT_NE(layer, other);

  other = layer;
  other.opacity = 0.5;
  EXPECT_NE(layer, other);

  other = layer;
  other.keyframes.push_back({.progress = 0});
  EXPECT_NE(layer, other);

  other = layer;
  other.blend_mode = BrushPaint::BlendMode::kXor;
  EXPECT_NE(layer, other);
}

TEST(BrushPaintTest, BrushPaintEqualAndNotEqual) {
  absl::StatusOr<Uri> uri1 = Uri::Parse("/texture:foo");
  ASSERT_EQ(uri1.status(), absl::OkStatus());
  absl::StatusOr<Uri> uri2 = Uri::Parse("/texture:bar");
  ASSERT_EQ(uri2.status(), absl::OkStatus());
  BrushPaint paint = {{{.color_texture_uri = *uri1}}};

  BrushPaint other = paint;
  EXPECT_EQ(paint, other);

  other = paint;
  other.texture_layers[0].color_texture_uri = *uri2;
  EXPECT_NE(paint, other);

  other = paint;
  other.texture_layers.clear();
  EXPECT_NE(paint, other);

  other = paint;
  other.texture_layers.push_back({.color_texture_uri = *uri2});
  EXPECT_NE(paint, other);
}

TEST(BrushPaintTest, StringifyTextureMapping) {
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureMapping::kWinding), "kWinding");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureMapping::kTiling), "kTiling");
  EXPECT_EQ(absl::StrCat(static_cast<BrushPaint::TextureMapping>(99)),
            "TextureMapping(99)");
}

TEST(BrushPaintTest, StringifyTextureOrigin) {
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureOrigin::kStrokeSpaceOrigin),
            "kStrokeSpaceOrigin");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureOrigin::kFirstStrokeInput),
            "kFirstStrokeInput");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureOrigin::kLastStrokeInput),
            "kLastStrokeInput");
  EXPECT_EQ(absl::StrCat(static_cast<BrushPaint::TextureOrigin>(99)),
            "TextureOrigin(99)");
}

TEST(BrushPaintTest, StringifyTextureSizeUnit) {
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureSizeUnit::kBrushSize),
            "kBrushSize");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureSizeUnit::kStrokeSize),
            "kStrokeSize");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureSizeUnit::kStrokeCoordinates),
            "kStrokeCoordinates");
  EXPECT_EQ(absl::StrCat(static_cast<BrushPaint::TextureSizeUnit>(99)),
            "TextureSizeUnit(99)");
}

TEST(BrushPaintTest, StringifyBlendMode) {
  EXPECT_EQ(absl::StrCat(BrushPaint::BlendMode::kModulate), "kModulate");
  EXPECT_EQ(absl::StrCat(BrushPaint::BlendMode::kDstIn), "kDstIn");
  EXPECT_EQ(absl::StrCat(BrushPaint::BlendMode::kDstOut), "kDstOut");
  EXPECT_EQ(absl::StrCat(BrushPaint::BlendMode::kSrcAtop), "kSrcAtop");
  EXPECT_EQ(absl::StrCat(BrushPaint::BlendMode::kSrcIn), "kSrcIn");
  EXPECT_EQ(absl::StrCat(BrushPaint::BlendMode::kSrcOver), "kSrcOver");
  EXPECT_EQ(absl::StrCat(BrushPaint::BlendMode::kSrc), "kSrc");
  EXPECT_EQ(absl::StrCat(static_cast<BrushPaint::BlendMode>(99)),
            "BlendMode(99)");
}

TEST(BrushPaintTest, StringifyTextureKeyFrame) {
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureKeyframe{}),
            "TextureKeyframe{progress=0}");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureKeyframe{.progress = 0.3}),
            "TextureKeyframe{progress=0.3}");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureKeyframe{
                .progress = 0.3, .size = std::optional<Vec>({4, 6})}),
            "TextureKeyframe{progress=0.3, size=<4, 6>}");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureKeyframe{
                .progress = 0.3,
                .size = std::optional<Vec>({4, 6}),
                .offset = std::optional<Vec>({2, 0.2})}),
            "TextureKeyframe{progress=0.3, size=<4, 6>, offset=<2, 0.2>}");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureKeyframe{
                .progress = 0.3,
                .size = std::optional<Vec>({4, 6}),
                .offset = std::optional<Vec>({2, 0.2}),
                .rotation = kHalfPi}),
            "TextureKeyframe{progress=0.3, size=<4, 6>, offset=<2, 0.2>, "
            "rotation=0.5π}");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureKeyframe{
                .progress = 0.3,
                .size = std::optional<Vec>({4, 6}),
                .offset = std::optional<Vec>({2, 0.2}),
                .rotation = kHalfPi,
                .opacity = 0.6}),
            "TextureKeyframe{progress=0.3, size=<4, 6>, offset=<2, 0.2>, "
            "rotation=0.5π, opacity=0.6}");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureKeyframe{
                .progress = 0.3,
                .offset = std::optional<Vec>({2, 0.2}),
                .opacity = 0.6}),
            "TextureKeyframe{progress=0.3, offset=<2, 0.2>, opacity=0.6}");
}

TEST(BrushPaintTest, StringifyTextureLayer) {
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureLayer{}),
            "TextureLayer{color_texture_uri=, mapping=kTiling, "
            "origin=kStrokeSpaceOrigin, size_unit=kStrokeCoordinates, "
            "size=<1, 1>, offset=<0, 0>, rotation=0π, size_jitter=<0, 0>, "
            "offset_jitter=<0, 0>, rotation_jitter=0π, opacity=1, "
            "keyframes={}, blend_mode=kModulate}");
  EXPECT_EQ(absl::StrCat(BrushPaint::TextureLayer{.color_texture_uri =
                                                      CreateTestTextureUri()}),
            "TextureLayer{color_texture_uri=/texture:test-texture, "
            "mapping=kTiling, origin=kStrokeSpaceOrigin, "
            "size_unit=kStrokeCoordinates, "
            "size=<1, 1>, offset=<0, 0>, rotation=0π, size_jitter=<0, 0>, "
            "offset_jitter=<0, 0>, rotation_jitter=0π, opacity=1, "
            "keyframes={}, blend_mode=kModulate}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint::TextureLayer{
          .color_texture_uri = CreateTestTextureUri(),
          .mapping = BrushPaint::TextureMapping::kWinding,
          .origin = BrushPaint::TextureOrigin::kFirstStrokeInput,
          .size_unit = BrushPaint::TextureSizeUnit::kBrushSize,
          .size = {3, 5},
          .offset = {2, 0.2},
          .rotation = kHalfPi,
          .size_jitter = {0.1, 0.2},
          .offset_jitter = {0.7, 0.3},
          .rotation_jitter = kPi / 8,
          .opacity = 0.6,
          .keyframes = {{.progress = 0.2,
                         .size = std::optional<Vec>({2, 5}),
                         .rotation = kPi / 8}},
          .blend_mode = BrushPaint::BlendMode::kDstIn}),
      "TextureLayer{color_texture_uri=/texture:test-texture, "
      "mapping=kWinding, origin=kFirstStrokeInput, size_unit=kBrushSize, "
      "size=<3, 5>, offset=<2, 0.2>, rotation=0.5π, size_jitter=<0.1, 0.2>, "
      "offset_jitter=<0.7, 0.3>, rotation_jitter=0.125π, opacity=0.6, "
      "keyframes={TextureKeyframe{progress=0.2, size=<2, 5>, "
      "rotation=0.125π}}, blend_mode=kDstIn}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint::TextureLayer{
          .color_texture_uri = CreateTestTextureUri(),
          .mapping = BrushPaint::TextureMapping::kWinding,
          .origin = BrushPaint::TextureOrigin::kLastStrokeInput,
          .size_unit = BrushPaint::TextureSizeUnit::kBrushSize,
          .size = {3, 5},
          .offset = {2, 0.2},
          .rotation = kHalfPi,
          .size_jitter = {0.1, 0.2},
          .offset_jitter = {0.7, 0.3},
          .rotation_jitter = kPi / 8,
          .opacity = 0.6,
          .keyframes = {{.progress = 0.2,
                         .size = std::optional<Vec>({2, 5}),
                         .rotation = kPi / 8},
                        {.progress = 0.4,
                         .offset = std::optional<Vec>({2, 0.2}),
                         .opacity = 0.4}},
          .blend_mode = BrushPaint::BlendMode::kSrcAtop}),
      "TextureLayer{color_texture_uri=/texture:test-texture, "
      "mapping=kWinding, origin=kLastStrokeInput, size_unit=kBrushSize, "
      "size=<3, 5>, offset=<2, 0.2>, rotation=0.5π, size_jitter=<0.1, 0.2>, "
      "offset_jitter=<0.7, 0.3>, rotation_jitter=0.125π, opacity=0.6, "
      "keyframes={TextureKeyframe{progress=0.2, size=<2, 5>, rotation=0.125π}, "
      "TextureKeyframe{progress=0.4, offset=<2, 0.2>, opacity=0.4}}, "
      "blend_mode=kSrcAtop}");
}

TEST(BrushPaintTest, StringifyBrushPaint) {
  EXPECT_EQ(absl::StrCat(BrushPaint{}), "BrushPaint{texture_layers={}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{.texture_layers = {{}}}),
      "BrushPaint{texture_layers={TextureLayer{color_texture_uri=, "
      "mapping=kTiling, origin=kStrokeSpaceOrigin, "
      "size_unit=kStrokeCoordinates, size=<1, 1>, offset=<0, 0>, "
      "rotation=0π, size_jitter=<0, 0>, offset_jitter=<0, 0>, "
      "rotation_jitter=0π, opacity=1, keyframes={}, blend_mode=kModulate}}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{
          .texture_layers = {{.color_texture_uri = CreateTestTextureUri()}}}),
      "BrushPaint{texture_layers={TextureLayer{color_texture_uri=/"
      "texture:test-texture, mapping=kTiling, "
      "origin=kStrokeSpaceOrigin, size_unit=kStrokeCoordinates, "
      "size=<1, 1>, offset=<0, 0>, rotation=0π, "
      "size_jitter=<0, 0>, "
      "offset_jitter=<0, 0>, rotation_jitter=0π, opacity=1, keyframes={}, "
      "blend_mode=kModulate}}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{
          .texture_layers = {{.color_texture_uri = CreateTestTextureUri(),
                              .mapping = BrushPaint::TextureMapping::kWinding,
                              .size_unit =
                                  BrushPaint::TextureSizeUnit::kBrushSize}}}),
      "BrushPaint{texture_layers={TextureLayer{color_texture_uri=/"
      "texture:test-texture, mapping=kWinding, origin=kStrokeSpaceOrigin, "
      "size_unit=kBrushSize, size=<1, 1>, offset=<0, 0>, rotation=0π, "
      "size_jitter=<0, 0>, offset_jitter=<0, 0>, rotation_jitter=0π, "
      "opacity=1, keyframes={}, blend_mode=kModulate}}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{
          .texture_layers = {{.color_texture_uri = CreateTestTextureUri(),
                              .mapping = BrushPaint::TextureMapping::kWinding,
                              .size_unit =
                                  BrushPaint::TextureSizeUnit::kBrushSize,
                              .size = {3, 5}}}}),
      "BrushPaint{texture_layers={TextureLayer{color_texture_uri=/"
      "texture:test-texture, mapping=kWinding, origin=kStrokeSpaceOrigin, "
      "size_unit=kBrushSize, size=<3, 5>, offset=<0, 0>, rotation=0π, "
      "size_jitter=<0, 0>, offset_jitter=<0, 0>, rotation_jitter=0π, "
      "opacity=1, keyframes={}, blend_mode=kModulate}}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{
          .texture_layers = {{.color_texture_uri = CreateTestTextureUri(),
                              .size = {3, 5}}}}),
      "BrushPaint{texture_layers={TextureLayer{color_texture_uri=/"
      "texture:test-texture, mapping=kTiling, origin=kStrokeSpaceOrigin, "
      "size_unit=kStrokeCoordinates, size=<3, 5>, offset=<0, 0>, rotation=0π, "
      "size_jitter=<0, 0>, offset_jitter=<0, 0>, rotation_jitter=0π, "
      "opacity=1, keyframes={}, blend_mode=kModulate}}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{
          .texture_layers = {{.color_texture_uri = CreateTestTextureUri(),
                              .size = {3, 5},
                              .offset = {2, 0.2}}}}),
      "BrushPaint{texture_layers={TextureLayer{color_texture_uri=/"
      "texture:test-texture, mapping=kTiling, origin=kStrokeSpaceOrigin, "
      "size_unit=kStrokeCoordinates, size=<3, 5>, offset=<2, 0.2>, "
      "rotation=0π, size_jitter=<0, 0>, offset_jitter=<0, 0>, "
      "rotation_jitter=0π, opacity=1, keyframes={}, blend_mode=kModulate}}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{
          .texture_layers = {{.color_texture_uri = CreateTestTextureUri(),
                              .size = {3, 5},
                              .offset = {2, 0.2},
                              .rotation = kHalfPi,
                              .opacity = 0.6}}}),
      "BrushPaint{texture_layers={TextureLayer{color_texture_uri=/"
      "texture:test-texture, mapping=kTiling, origin=kStrokeSpaceOrigin, "
      "size_unit=kStrokeCoordinates, size=<3, 5>, offset=<2, 0.2>, "
      "rotation=0.5π, size_jitter=<0, 0>, offset_jitter=<0, 0>, "
      "rotation_jitter=0π, opacity=0.6, keyframes={}, blend_mode=kModulate}}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{
          .texture_layers = {{.color_texture_uri = CreateTestTextureUri(),
                              .mapping = BrushPaint::TextureMapping::kWinding,
                              .size_unit =
                                  BrushPaint::TextureSizeUnit::kBrushSize,
                              .size = {3, 5},
                              .offset = {2, 0.2},
                              .blend_mode = BrushPaint::BlendMode::kSrcIn}}}),
      "BrushPaint{texture_layers={TextureLayer{color_texture_uri=/"
      "texture:test-texture, mapping=kWinding, origin=kStrokeSpaceOrigin, "
      "size_unit=kBrushSize, size=<3, 5>, offset=<2, 0.2>, rotation=0π, "
      "size_jitter=<0, 0>, offset_jitter=<0, 0>, rotation_jitter=0π, "
      "opacity=1, keyframes={}, blend_mode=kSrcIn}}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{
          .texture_layers = {{.color_texture_uri = CreateTestTextureUri(),
                              .mapping = BrushPaint::TextureMapping::kWinding,
                              .size_unit =
                                  BrushPaint::TextureSizeUnit::kBrushSize,
                              .size = {3, 5},
                              .offset = {2, 0.2},
                              .rotation = kHalfPi,
                              .opacity = 0.6}}}),
      "BrushPaint{texture_layers={TextureLayer{color_texture_uri=/"
      "texture:test-texture, mapping=kWinding, origin=kStrokeSpaceOrigin, "
      "size_unit=kBrushSize, size=<3, 5>, offset=<2, 0.2>, rotation=0.5π, "
      "size_jitter=<0, 0>, offset_jitter=<0, 0>, rotation_jitter=0π, "
      "opacity=0.6, keyframes={}, blend_mode=kModulate}}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{
          .texture_layers = {{.color_texture_uri = CreateTestTextureUri(),
                              .mapping = BrushPaint::TextureMapping::kWinding,
                              .size_unit =
                                  BrushPaint::TextureSizeUnit::kBrushSize,
                              .size = {3, 5},
                              .offset = {2, 0.2},
                              .rotation = kHalfPi,
                              .size_jitter = {0.1, 0.2},
                              .offset_jitter = {0.7, 0.3},
                              .rotation_jitter = kPi / 8,
                              .opacity = 0.6,
                              .blend_mode = BrushPaint::BlendMode::kSrcIn}}}),
      "BrushPaint{texture_layers={TextureLayer{color_texture_uri=/"
      "texture:test-texture, mapping=kWinding, origin=kStrokeSpaceOrigin, "
      "size_unit=kBrushSize, size=<3, 5>, offset=<2, 0.2>, rotation=0.5π, "
      "size_jitter=<0.1, 0.2>, offset_jitter=<0.7, 0.3>, "
      "rotation_jitter=0.125π, opacity=0.6, keyframes={}, "
      "blend_mode=kSrcIn}}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{
          .texture_layers =
              {{.color_texture_uri = CreateTestTextureUri(),
                .mapping = BrushPaint::TextureMapping::kWinding,
                .size_unit = BrushPaint::TextureSizeUnit::kBrushSize,
                .size = {3, 5},
                .offset = {2, 0.2},
                .rotation = kHalfPi,
                .size_jitter = {0.1, 0.2},
                .offset_jitter = {0.7, 0.3},
                .rotation_jitter = kPi / 8,
                .opacity = 0.6,
                .keyframes = {{.progress = 0.3,
                               .size = std::optional<Vec>({4, 6}),
                               .offset = std::optional<Vec>({2, 0.2}),
                               .rotation = kHalfPi,
                               .opacity = 0.6}}}}}),
      "BrushPaint{texture_layers={TextureLayer{color_texture_uri=/"
      "texture:test-texture, mapping=kWinding, origin=kStrokeSpaceOrigin, "
      "size_unit=kBrushSize, size=<3, 5>, offset=<2, 0.2>, rotation=0.5π, "
      "size_jitter=<0.1, 0.2>, offset_jitter=<0.7, 0.3>, "
      "rotation_jitter=0.125π, opacity=0.6, "
      "keyframes={TextureKeyframe{progress=0.3, size=<4, 6>, offset=<2, 0.2>, "
      "rotation=0.5π, opacity=0.6}}, blend_mode=kModulate}}}");
  EXPECT_EQ(
      absl::StrCat(BrushPaint{
          .texture_layers =
              {{.color_texture_uri = CreateTestTextureUri(),
                .mapping = BrushPaint::TextureMapping::kWinding,
                .size_unit = BrushPaint::TextureSizeUnit::kBrushSize,
                .size = {3, 5},
                .offset = {2, 0.2},
                .rotation = kHalfPi,
                .size_jitter = {0.1, 0.2},
                .offset_jitter = {0.7, 0.3},
                .rotation_jitter = kPi / 8,
                .opacity = 0.6,
                .blend_mode = BrushPaint::BlendMode::kSrcIn},
               {.color_texture_uri = CreateTestTextureUri(),
                .mapping = BrushPaint::TextureMapping::kTiling,
                .size_unit = BrushPaint::TextureSizeUnit::kStrokeSize,
                .size = {1, 4},
                .opacity = 0.7,
                .keyframes = {{.progress = 0.2,
                               .size = std::optional<Vec>({2, 5}),
                               .rotation = kPi / 8},
                              {.progress = 0.4,
                               .offset = std::optional<Vec>({2, 0.2}),
                               .opacity = 0.4}},
                .blend_mode = BrushPaint::BlendMode::kDstIn}}}),
      "BrushPaint{texture_layers={TextureLayer{color_texture_uri=/"
      "texture:test-texture, mapping=kWinding, origin=kStrokeSpaceOrigin, "
      "size_unit=kBrushSize, size=<3, 5>, offset=<2, 0.2>, rotation=0.5π, "
      "size_jitter=<0.1, 0.2>, offset_jitter=<0.7, 0.3>, "
      "rotation_jitter=0.125π, opacity=0.6, keyframes={}, blend_mode=kSrcIn}, "
      "TextureLayer{color_texture_uri=/texture:test-texture, mapping=kTiling, "
      "origin=kStrokeSpaceOrigin, size_unit=kStrokeSize, size=<1, 4>, "
      "offset=<0, 0>, rotation=0π, size_jitter=<0, 0>, offset_jitter=<0, 0>, "
      "rotation_jitter=0π, opacity=0.7, "
      "keyframes={TextureKeyframe{progress=0.2, size=<2, 5>, rotation=0.125π}, "
      "TextureKeyframe{progress=0.4, offset=<2, 0.2>, opacity=0.4}}, "
      "blend_mode=kDstIn}}}");
}

TEST(BrushPaintTest, InvalidTextureLayerRotation) {
  absl::Status status = brush_internal::ValidateBrushPaint(BrushPaint{
      .texture_layers = {{.color_texture_uri = CreateTestTextureUri(),
                          .rotation = Angle::Radians(kInfinity)}}});
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("rotation` must be finite"));

  status = brush_internal::ValidateBrushPaint(BrushPaint{
      .texture_layers = {{.color_texture_uri = CreateTestTextureUri(),
                          .rotation = Angle::Radians(kNan)}}});
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("rotation` must be finite"));
}

TEST(BrushPaintTest, InvalidTextureLayerRotationJitter) {
  absl::Status status = brush_internal::ValidateBrushPaint(BrushPaint{
      .texture_layers = {{.color_texture_uri = CreateTestTextureUri(),
                          .rotation_jitter = Angle::Radians(kInfinity)}}});
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("rotation_jitter` must be finite"));

  status = brush_internal::ValidateBrushPaint(BrushPaint{
      .texture_layers = {{.color_texture_uri = CreateTestTextureUri(),
                          .rotation_jitter = Angle::Radians(kNan)}}});
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(status.message(), HasSubstr("rotation_jitter` must be finite"));
}

}  // namespace
}  // namespace ink
