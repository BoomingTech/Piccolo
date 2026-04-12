// Standalone test and benchmark for fast_float + OBJ-specific float parser.
// Build: cmake -B build -DCMAKE_BUILD_TYPE=Release . && cmake --build build
// Run:   ./build/test_parse_fp

#include "obj_parse_float.h"

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Reference: tinyobjloader's existing tryParseDouble (for comparison)
// ---------------------------------------------------------------------------
#define IS_DIGIT(x) (static_cast<unsigned int>((x) - '0') < 10)

static bool tryParseDouble_legacy(const char *s, const char *s_end,
                                  double *result) {
  if (s >= s_end) return false;

  double mantissa = 0.0;
  int exponent = 0;
  char sign = '+';
  char exp_sign = '+';
  const char *curr = s;
  int read = 0;
  bool end_not_reached = false;
  bool leading_decimal_dots = false;

  if (*curr == '+' || *curr == '-') {
    sign = *curr;
    curr++;
    if ((curr != s_end) && (*curr == '.')) {
      leading_decimal_dots = true;
    }
  } else if (IS_DIGIT(*curr)) {
  } else if (*curr == '.') {
    leading_decimal_dots = true;
  } else {
    goto fail;
  }

  end_not_reached = (curr != s_end);
  if (!leading_decimal_dots) {
    while (end_not_reached && IS_DIGIT(*curr)) {
      mantissa *= 10;
      mantissa += static_cast<int>(*curr - 0x30);
      curr++;
      read++;
      end_not_reached = (curr != s_end);
    }
    if (read == 0) goto fail;
  }

  if (!end_not_reached) goto assemble;

  if (*curr == '.') {
    curr++;
    read = 1;
    end_not_reached = (curr != s_end);
    while (end_not_reached && IS_DIGIT(*curr)) {
      static const double pow_lut[] = {
          1.0, 0.1, 0.01, 0.001, 0.0001, 0.00001, 0.000001, 0.0000001,
      };
      const int lut_entries = sizeof pow_lut / sizeof pow_lut[0];
      mantissa += static_cast<int>(*curr - 0x30) *
                  (read < lut_entries ? pow_lut[read] : std::pow(10.0, -read));
      read++;
      curr++;
      end_not_reached = (curr != s_end);
    }
  } else if (*curr == 'e' || *curr == 'E') {
  } else {
    goto assemble;
  }

  if (!end_not_reached) goto assemble;

  if (*curr == 'e' || *curr == 'E') {
    curr++;
    end_not_reached = (curr != s_end);
    if (end_not_reached && (*curr == '+' || *curr == '-')) {
      exp_sign = *curr;
      curr++;
    } else if (IS_DIGIT(*curr)) {
    } else {
      goto fail;
    }

    read = 0;
    end_not_reached = (curr != s_end);
    while (end_not_reached && IS_DIGIT(*curr)) {
      if (exponent > (2147483647 / 10)) goto fail;
      exponent *= 10;
      exponent += static_cast<int>(*curr - 0x30);
      curr++;
      read++;
      end_not_reached = (curr != s_end);
    }
    exponent *= (exp_sign == '+' ? 1 : -1);
    if (read == 0) goto fail;
  }

assemble:
  *result =
      (sign == '+' ? 1 : -1) *
      (exponent ? std::ldexp(mantissa * std::pow(5.0, exponent), exponent)
                : mantissa);
  return true;
fail:
  return false;
}

#undef IS_DIGIT

// ---------------------------------------------------------------------------
// Test helpers
// ---------------------------------------------------------------------------
static int g_failures = 0;

#define CHECK(cond, ...)                                                       \
  do {                                                                         \
    if (!(cond)) {                                                             \
      printf("FAIL %s:%d: ", __FILE__, __LINE__);                              \
      printf(__VA_ARGS__);                                                     \
      printf("\n");                                                            \
      g_failures++;                                                            \
    }                                                                          \
  } while (0)

// ---------------------------------------------------------------------------
// 1. Basic fast_float correctness
// ---------------------------------------------------------------------------
static int test_fast_float_basic() {
  struct TestCase {
    const char *input;
    double expected;
    bool should_succeed;
  };

  const TestCase cases[] = {
      {"0", 0.0, true},
      {"1", 1.0, true},
      {"-1", -1.0, true},
      {"123", 123.0, true},
      {"-456", -456.0, true},
      {"3.14", 3.14, true},
      {"-2.718", -2.718, true},
      {"0.5", 0.5, true},
      {".5", 0.5, true},
      {"-.5", -0.5, true},
      {"1e10", 1e10, true},
      {"1E10", 1E10, true},
      {"1.5e3", 1.5e3, true},
      {"-2.5e-4", -2.5e-4, true},
      {"1e0", 1.0, true},
      {"1e+0", 1.0, true},
      {"1e-0", 1.0, true},
      {"1e308", 1e308, true},
      {"5e-324", 5e-324, true},
      {"1.7976931348623157e308", 1.7976931348623157e308, true},
      {"0.123456", 0.123456, true},
      {"-0.987654", -0.987654, true},
      {"1.000000", 1.0, true},
      {"0.000000", 0.0, true},
  };

  int before = g_failures;
  const int n = sizeof(cases) / sizeof(cases[0]);

  for (int i = 0; i < n; i++) {
    const TestCase &tc = cases[i];
    double val = 0.0;
    const char *first = tc.input;
    const char *last = first + std::strlen(first);
    auto result = fast_float::from_chars(first, last, val);

    bool ok = (result.ec == std::errc());
    CHECK(ok == tc.should_succeed, "[%d] \"%s\": expected %s, got %s", i,
          tc.input, tc.should_succeed ? "success" : "failure",
          ok ? "success" : "failure");

    if (ok && tc.expected != 0.0) {
      double rel_err = std::fabs((val - tc.expected) / tc.expected);
      CHECK(rel_err <= 1e-15, "[%d] \"%s\": expected %.17g, got %.17g", i,
            tc.input, tc.expected, val);
    } else if (ok && tc.expected == 0.0) {
      CHECK(val == 0.0, "[%d] \"%s\": expected 0.0, got %.17g", i, tc.input,
            val);
    }
  }

  return g_failures - before;
}

// ---------------------------------------------------------------------------
// 2. OBJ wrapper: leading '+' sign
// ---------------------------------------------------------------------------
static int test_leading_plus() {
  int before = g_failures;

  struct TestCase {
    const char *input;
    double expected;
  };

  const TestCase cases[] = {
      {"+1.0", 1.0},     {"+0", 0.0},       {"+0.5", 0.5},
      {"+123", 123.0},   {"+1e5", 1e5},      {"+.5", 0.5},
      {"+1.5e-3", 1.5e-3},
  };

  const int n = sizeof(cases) / sizeof(cases[0]);
  for (int i = 0; i < n; i++) {
    double val = -999.0;
    const char *end = NULL;
    bool ok = obj::parseFloat(cases[i].input, &val, &end);
    CHECK(ok, "leading+ [%d] \"%s\": parse failed", i, cases[i].input);
    if (ok && cases[i].expected != 0.0) {
      double rel_err = std::fabs((val - cases[i].expected) / cases[i].expected);
      CHECK(rel_err <= 1e-15, "leading+ [%d] \"%s\": expected %.17g, got %.17g",
            i, cases[i].input, cases[i].expected, val);
    } else if (ok) {
      CHECK(val == 0.0, "leading+ [%d] \"%s\": expected 0.0, got %.17g", i,
            cases[i].input, val);
    }
  }

  return g_failures - before;
}

// ---------------------------------------------------------------------------
// 3. nan/inf parsing with default replacements
// ---------------------------------------------------------------------------
static int test_nan_inf_defaults() {
  int before = g_failures;

  double dmax = (std::numeric_limits<double>::max)();
  double dmin = std::numeric_limits<double>::lowest();

  struct TestCase {
    const char *input;
    double expected;
  };

  const TestCase cases[] = {
      // nan variants
      {"nan", 0.0},
      {"NaN", 0.0},
      {"NAN", 0.0},
      {"+nan", 0.0},
      {"-nan", 0.0},

      // inf variants
      {"inf", dmax},
      {"Inf", dmax},
      {"INF", dmax},
      {"infinity", dmax},
      {"Infinity", dmax},
      {"INFINITY", dmax},
      {"+inf", dmax},
      {"+Inf", dmax},
      {"+infinity", dmax},

      // -inf variants
      {"-inf", dmin},
      {"-Inf", dmin},
      {"-INF", dmin},
      {"-infinity", dmin},
      {"-Infinity", dmin},
      {"-INFINITY", dmin},
  };

  const int n = sizeof(cases) / sizeof(cases[0]);
  for (int i = 0; i < n; i++) {
    double val = -12345.0;
    const char *end = NULL;
    bool ok = obj::parseFloat(cases[i].input, &val, &end);
    CHECK(ok, "nan_inf [%d] \"%s\": parse failed", i, cases[i].input);
    CHECK(val == cases[i].expected,
          "nan_inf [%d] \"%s\": expected %.17g, got %.17g", i, cases[i].input,
          cases[i].expected, val);
  }

  return g_failures - before;
}

// ---------------------------------------------------------------------------
// 4. nan/inf with custom replacement values
// ---------------------------------------------------------------------------
static int test_nan_inf_custom() {
  int before = g_failures;

  obj::ParseOptions<double> opts;
  opts.nan_value = -1.0;
  opts.inf_value = 9999.0;
  opts.neg_inf_value = -9999.0;

  {
    double val = 0.0;
    bool ok = obj::parseFloat("nan", &val, opts);
    CHECK(ok && val == -1.0, "custom nan: expected -1.0, got %.17g", val);
  }
  {
    double val = 0.0;
    bool ok = obj::parseFloat("NaN", &val, opts);
    CHECK(ok && val == -1.0, "custom NaN: expected -1.0, got %.17g", val);
  }
  {
    double val = 0.0;
    bool ok = obj::parseFloat("inf", &val, opts);
    CHECK(ok && val == 9999.0, "custom inf: expected 9999.0, got %.17g", val);
  }
  {
    double val = 0.0;
    bool ok = obj::parseFloat("-inf", &val, opts);
    CHECK(ok && val == -9999.0, "custom -inf: expected -9999.0, got %.17g",
          val);
  }
  {
    double val = 0.0;
    bool ok = obj::parseFloat("infinity", &val, opts);
    CHECK(ok && val == 9999.0, "custom infinity: expected 9999.0, got %.17g",
          val);
  }
  {
    double val = 0.0;
    bool ok = obj::parseFloat("-infinity", &val, opts);
    CHECK(ok && val == -9999.0,
          "custom -infinity: expected -9999.0, got %.17g", val);
  }

  // float version
  obj::ParseOptions<float> fopts;
  fopts.nan_value = -2.0f;
  fopts.inf_value = 1e30f;
  fopts.neg_inf_value = -1e30f;
  {
    float fval = 0.0f;
    bool ok = obj::parseFloat("nan", &fval, fopts);
    CHECK(ok && fval == -2.0f, "custom float nan: expected -2.0, got %f", fval);
  }
  {
    float fval = 0.0f;
    bool ok = obj::parseFloat("inf", &fval, fopts);
    CHECK(ok && fval == 1e30f, "custom float inf: expected 1e30, got %g", fval);
  }

  return g_failures - before;
}

// ---------------------------------------------------------------------------
// 5. Pointer advancement / whitespace handling
// ---------------------------------------------------------------------------
static int test_pointer_advance() {
  int before = g_failures;

  // parseFloat skips leading whitespace
  {
    double val = 0.0;
    const char *end = NULL;
    bool ok = obj::parseFloat("  1.5 rest", &val, &end);
    CHECK(ok, "ws: parse failed");
    CHECK(val == 1.5, "ws: expected 1.5, got %.17g", val);
    CHECK(end && *end == ' ', "ws: expected end at space before 'rest'");
  }

  // Tab whitespace
  {
    double val = 0.0;
    const char *end = NULL;
    bool ok = obj::parseFloat("\t3.14\tnext", &val, &end);
    CHECK(ok, "tab ws: parse failed");
    CHECK(std::fabs(val - 3.14) < 1e-15, "tab ws: expected 3.14, got %.17g",
          val);
    CHECK(end && *end == '\t', "tab ws: expected end at tab");
  }

  // nan pointer advancement
  {
    double val = -1.0;
    const char *end = NULL;
    bool ok = obj::parseFloat("nan rest", &val, &end);
    CHECK(ok, "nan ptr: parse failed");
    CHECK(val == 0.0, "nan ptr: expected 0.0, got %.17g", val);
    CHECK(end && *end == ' ', "nan ptr: expected end at space");
  }

  // inf pointer advancement
  {
    double val = 0.0;
    const char *end = NULL;
    bool ok = obj::parseFloat("infinity rest", &val, &end);
    CHECK(ok, "inf ptr: parse failed");
    CHECK(end && *end == ' ', "inf ptr: expected end at space");
  }

  // parseFloatRange
  {
    double val = 0.0;
    const char *end = NULL;
    const char *s = "+1.5e2xxx";
    bool ok = obj::parseFloatRange(s, s + 6, &val, &end);
    CHECK(ok, "range: parse failed");
    CHECK(val == 150.0, "range: expected 150.0, got %.17g", val);
  }

  // parseFloatRange with nan
  {
    double val = -1.0;
    const char *end = NULL;
    const char *s = "nan";
    bool ok = obj::parseFloatRange(s, s + 3, &val, &end);
    CHECK(ok, "range nan: parse failed");
    CHECK(val == 0.0, "range nan: expected 0.0, got %.17g", val);
  }

  return g_failures - before;
}

// ---------------------------------------------------------------------------
// 6. Edge cases / error handling
// ---------------------------------------------------------------------------
static int test_edge_cases() {
  int before = g_failures;

  // Empty string
  {
    double val = 999.0;
    const char *end = NULL;
    bool ok = obj::parseFloat("", &val, &end);
    CHECK(!ok, "empty: should fail");
  }

  // Whitespace only
  {
    double val = 999.0;
    const char *end = NULL;
    bool ok = obj::parseFloat("   ", &val, &end);
    CHECK(!ok, "ws-only: should fail");
  }

  // Just '+'
  {
    double val = 999.0;
    const char *end = NULL;
    bool ok = obj::parseFloat("+", &val, &end);
    CHECK(!ok, "bare-plus: should fail");
  }

  // Normal negative still works
  {
    double val = 0.0;
    bool ok = obj::parseFloat("-3.14", &val);
    CHECK(ok && std::fabs(val - (-3.14)) < 1e-15,
          "negative: expected -3.14, got %.17g", val);
  }

  // "nana" should parse "nan" and stop
  {
    double val = -1.0;
    const char *end = NULL;
    // Use range-based to see where it stops
    const char *s = "nana";
    bool ok = obj::parseFloatRange(s, s + 4, &val, &end);
    CHECK(ok, "nana: parse should succeed (match 'nan')");
    CHECK(val == 0.0, "nana: expected 0.0, got %.17g", val);
    CHECK(end == s + 3, "nana: expected end at s+3, got s+%d", (int)(end - s));
  }

  // "info" should parse "inf" and stop at 'o'
  {
    double val = 0.0;
    const char *end = NULL;
    const char *s = "info";
    bool ok = obj::parseFloatRange(s, s + 4, &val, &end);
    CHECK(ok, "info: parse should succeed (match 'inf')");
    double dmax = (std::numeric_limits<double>::max)();
    CHECK(val == dmax, "info: expected max, got %.17g", val);
    CHECK(end == s + 3, "info: expected end at s+3, got s+%d", (int)(end - s));
  }

  return g_failures - before;
}

// ---------------------------------------------------------------------------
// 7. Accuracy comparison: fast_float vs legacy tryParseDouble vs strtod
// ---------------------------------------------------------------------------
static int test_accuracy() {
  const char *hard_cases[] = {
      "2.2250738585072014e-308",
      "2.2250738585072011e-308",
      "1.00000000000000011102230246251565404236316680908203125",
      "0.3",
      "0.1",
      "0.2",
      "7205759403792794e-1",
      "922337203685477.5807",
      "1e23",
  };

  int before = g_failures;
  const int n = sizeof(hard_cases) / sizeof(hard_cases[0]);

  printf("--- Accuracy comparison ---\n");
  printf("%-55s %22s %22s %22s\n", "Input", "strtod", "fast_float", "legacy");

  for (int i = 0; i < n; i++) {
    const char *s = hard_cases[i];
    size_t len = std::strlen(s);

    double ref = std::strtod(s, NULL);

    double ff_val = 0.0;
    fast_float::from_chars(s, s + len, ff_val);

    double lg_val = 0.0;
    tryParseDouble_legacy(s, s + len, &lg_val);

    printf("%-55s %22.17g %22.17g %22.17g\n", s, ref, ff_val, lg_val);

    CHECK(ref == ff_val, "accuracy [%d] \"%s\": fast_float mismatch vs strtod",
          i, s);
    if (ref != lg_val) {
      printf("  ^^ legacy MISMATCH vs strtod (expected)\n");
    }
  }

  return g_failures - before;
}

// ---------------------------------------------------------------------------
// Benchmark
// ---------------------------------------------------------------------------
static void benchmark() {
  const int N = 1000000;
  std::vector<std::string> data;
  data.reserve(N);

  const char *patterns[] = {
      "0.123456",   "-0.987654",  "1.000000",   "0.000000",  "123.456",
      "-789.012",   "0.5",        "-0.5",        "1e-5",      "3.14159265",
      "2.71828182", "-0.0001234", "999.999999",  "0.333333",  "-1.414213",
      "+0.5",       "+123.456",   "+1e-5",  // OBJ '+' prefix
  };
  const int np = sizeof(patterns) / sizeof(patterns[0]);
  for (int i = 0; i < N; i++) {
    data.push_back(patterns[i % np]);
  }

  double sum;
  typedef std::chrono::high_resolution_clock Clock;

  // Benchmark obj::parseFloat (fast_float + OBJ wrapper)
  sum = 0.0;
  auto t0 = Clock::now();
  for (int i = 0; i < N; i++) {
    double val = 0.0;
    obj::parseFloat(data[i].c_str(), &val);
    sum += val;
  }
  auto t1 = Clock::now();
  double obj_ms =
      std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() /
      1000.0;
  printf("obj::parseFloat: %8.2f ms  (sum=%.6f, %d values)\n", obj_ms, sum, N);

  // Benchmark legacy tryParseDouble
  sum = 0.0;
  t0 = Clock::now();
  for (int i = 0; i < N; i++) {
    double val = 0.0;
    const char *s = data[i].c_str();
    size_t len = data[i].size();
    // Skip whitespace for fair comparison
    while (*s == ' ' || *s == '\t') { ++s; --len; }
    const char *e = s + len;
    while (e > s && (*(e-1) == ' ' || *(e-1) == '\t')) --e;
    tryParseDouble_legacy(s, e, &val);
    sum += val;
  }
  t1 = Clock::now();
  double lg_ms =
      std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() /
      1000.0;
  printf("legacy:          %8.2f ms  (sum=%.6f, %d values)\n", lg_ms, sum, N);

  // Benchmark strtod
  sum = 0.0;
  t0 = Clock::now();
  for (int i = 0; i < N; i++) {
    sum += std::strtod(data[i].c_str(), NULL);
  }
  t1 = Clock::now();
  double sd_ms =
      std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() /
      1000.0;
  printf("strtod:          %8.2f ms  (sum=%.6f, %d values)\n", sd_ms, sum, N);

  printf("\nSpeedup obj::parseFloat vs legacy:  %.2fx\n", lg_ms / obj_ms);
  printf("Speedup obj::parseFloat vs strtod:  %.2fx\n", sd_ms / obj_ms);

  // Benchmark nan/inf mix
  printf("\n--- nan/inf mixed benchmark (100K) ---\n");
  const int M = 100000;
  std::vector<std::string> mixed;
  mixed.reserve(M);
  const char *mixed_patterns[] = {
      "1.5",  "-2.3",  "nan",  "inf",  "-inf",
      "0.5",  "NaN",   "Inf",  "+1.0", "infinity",
  };
  const int mp = sizeof(mixed_patterns) / sizeof(mixed_patterns[0]);
  for (int i = 0; i < M; i++) {
    mixed.push_back(mixed_patterns[i % mp]);
  }

  sum = 0.0;
  t0 = Clock::now();
  for (int i = 0; i < M; i++) {
    double val = 0.0;
    obj::parseFloat(mixed[i].c_str(), &val);
    sum += val;
  }
  t1 = Clock::now();
  double mix_ms =
      std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() /
      1000.0;
  printf("obj::parseFloat (mixed): %8.2f ms  (sum=%.6f, %d values)\n", mix_ms,
         sum, M);
}

// ---------------------------------------------------------------------------
int main() {
  int section_fails;

  printf("=== 1. fast_float basic correctness ===\n");
  section_fails = test_fast_float_basic();
  printf("  %s\n\n", section_fails == 0 ? "PASSED" : "FAILED");

  printf("=== 2. Leading '+' sign ===\n");
  section_fails = test_leading_plus();
  printf("  %s\n\n", section_fails == 0 ? "PASSED" : "FAILED");

  printf("=== 3. nan/inf with default replacements ===\n");
  section_fails = test_nan_inf_defaults();
  printf("  %s\n\n", section_fails == 0 ? "PASSED" : "FAILED");

  printf("=== 4. nan/inf with custom replacements ===\n");
  section_fails = test_nan_inf_custom();
  printf("  %s\n\n", section_fails == 0 ? "PASSED" : "FAILED");

  printf("=== 5. Pointer advancement / whitespace ===\n");
  section_fails = test_pointer_advance();
  printf("  %s\n\n", section_fails == 0 ? "PASSED" : "FAILED");

  printf("=== 6. Edge cases ===\n");
  section_fails = test_edge_cases();
  printf("  %s\n\n", section_fails == 0 ? "PASSED" : "FAILED");

  printf("=== 7. Accuracy comparison ===\n");
  section_fails = test_accuracy();
  printf("  %s\n\n", section_fails == 0 ? "PASSED" : "FAILED");

  printf("=== Benchmark (1M parses) ===\n");
  benchmark();

  printf("\n");
  if (g_failures == 0) {
    printf("ALL TESTS PASSED.\n");
    return 0;
  } else {
    printf("%d TEST(S) FAILED.\n", g_failures);
    return 1;
  }
}
