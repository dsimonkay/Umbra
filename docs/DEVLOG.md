# Devlog

Newest entry on top. Keep entries short. The most important line is **Next tiny step**: something
concrete you can finish in 30 minutes, so the next session starts with doing instead of remembering.

Template:

```
## YYYY-MM-DD — Mx: short title
- Done: ...
- Learned: ...
- Interview story: ... (only if something interesting happened: a bug, a design decision)
- Next tiny step: ...
```

---

## 2026-10-28 — M2: Catch2, the first real test (`Scene::Hit`)

- Done:
  - By Claude (plumbing): Catch2 v3.16.1 in `cmake/Dependencies.cmake` (FetchContent with `SYSTEM`, so our
    warning flags skip its headers). `tests/` builds `umbra_tests` (links `umbra_core` and
    `Catch2::Catch2WithMain`), one `<name>_test.cpp` per core file. `catch_discover_tests` makes every
    `TEST_CASE` its own ctest test, next to the old `cli_smoke`. Checked with GCC 13: `debug` and `asan-ubsan`
    build without warnings. A deliberately wrong check fails with its values,
    `0.5f is within 0.00000099999999748 of 0.60000002384185791` (the matchers compute in `double`, so a float
    literal shows its real value).
  - `tests/scene_test.cpp`: one `TEST_CASE` with five `SECTION`s. An empty scene hits nothing; a sphere beside
    the ray is not hit; a sphere on the ray is hit at `t = 0.5`; of two spheres on the ray the nearer one is
    hit (`t = 1.5`), in both list orders.
  - Seen it fail: with the line that narrows `valid_interval.max` commented out, the "nearer one added first"
    section fails: `t` is 11.5 instead of 1.5, the far sphere won.
  - Moved the repository from `/mnt/d/code/Umbra` to the WSL home: the builds are much faster there.
- Decided: Catch2, not doctest. I already use it in a private project (and GTest with Bazel at work), and this
  project is about the ray tracer, not about one more test framework. Its float matchers (`WithinAbs`,
  `WithinRel`, `WithinULP`, combinable with `||`) make the tolerance of every check explicit.
- Learned:
  - My first two-sphere section put the far sphere first, and it passed *with* yesterday's bug: the broken
    loop keeps the last hit, and the last one was the near sphere. Only the other order catches it. A test is
    worth something once I have seen it fail.
  - A `SECTION` is not a ctest test. Catch2 finds the sections by running the `TEST_CASE`: it runs the body
    from the top once per leaf section (that's why every section gets a fresh `scene`). `--list-tests`, which
    `catch_discover_tests` reads, only knows the `TEST_CASE`s. To see the sections:
    `./build/debug/umbra_tests "[scene]" -s`. (Catch2 has fixtures too, `TEST_CASE_METHOD` ≈ GTest's `TEST_F`;
    not needed here.)
  - WSL reaches the Windows drives (`/mnt/c`, `/mnt/d`) through a network file system (9P), and a build
    touches thousands of files. Keep the repository in the Linux file system.
  - The exact tangent ray, `(0.5, 0, 0) → (0, 0, −1)` against the sphere at `(0, 0, −1)` with r = 0.5: the
    discriminant is exactly 0, so it hits at `t = 1`. But `direction · n` is exactly 0 as well, so
    `front_face` is false and the normal points *into* the sphere, `(−1, 0, 0)`, although the ray comes from
    outside. Rare in practice, but a test has to state something: `<` or `<=` in `SetFaceNormal`?
  - `kVector3Epsilon = 1e-6` is absolute. Floats are 9.5e-7 apart at 8 and 1.9e-6 apart at 16, so above 16 it
    means "bit-exact". In Image 5 the `t` near the horizon is about 10, and M5's ground sphere has radius 1000.
- Interview story: "Have you written a test that passed against the bug it was meant to catch?" Yes: the
  regression test for my closest-hit bug. The bug made the last sphere in the list win, and my test happened
  to put the nearer sphere last, so the broken code passed. It came out in review: with the bug put back, the
  test stayed green. Since then, every new test has to fail once, on purpose, before I trust it.
- Next tiny step: `tests/sphere_test.cpp` (add it to `tests/CMakeLists.txt`), tag `[sphere]`, the sphere at
  `(0, 0, −1)` with r = 0.5. Three sections first: a hit from the origin along −z (`t = 0.5`, point
  `(0, 0, −0.5)`, normal `(0, 0, 1)`, `front_face`); the same ray with direction `(0, 0, −2)` (`t = 0.25`,
  the same point); the sphere moved to z = +1, behind the ray: no hit with `Interval{0.001f}`, but with
  `Interval{}`, as in `scene_test.cpp`, it *is* hit, at `t = −1.5`. Then the ray starting inside, and the
  tangent case after deciding `<` or `<=` in `SetFaceNormal`.

## 2026-10-27 — M2: `Scene::Hit` and the ground sphere (RTiOW Image 5)

- Done:
  - M2 step 4 (RTiOW 6.5): `Scene::Hit(ray, interval)` in the new `core/scene.cpp` returns the closest hit. It
    goes over `spheres` and shrinks the interval's `max` to the `t` of each hit (the book's `closest_so_far`),
    so a later sphere counts only if it is nearer. `RayColor` calls `scene.Hit(...)` instead of asking
    `spheres[0]`.
  - The ground sphere, `Sphere{Point3{0, -100.5f, -1}, 100}`, in both `main`s: RTiOW Image 5.
  - `front_face` (RTiOW 6.4) was already in the previous commit: `HitRecord::SetFaceNormal` turns the normal
    against the ray. The previous entry still listed it as open. With that, M2 step 4 is done.
  - The unit-normal assert in `SetFaceNormal` uses a tolerance now (`|n · n − 1| < 1e-6`). With `== 1.0f`, a
    build with asserts aborted at the first pixel that hit the sphere. Release builds hide it (`-DNDEBUG`).
- Learned:
  - The first `Scene::Hit` drew the ground over the lower part of the small sphere. `closest_hit_so_far`
    started at `max` and was never updated, so every hit passed `t < closest_hit_so_far`. The *last* sphere
    in the list that was hit won, not the nearest one. 30 144 pixels were wrong: rows 294–425, the part of
    the small sphere below the horizon.
  - With the ground *first* in the list, the broken version renders exactly the right picture (0 pixels
    differ). A "closest" search can look right because of the order of the objects alone.
  - The picture can be checked with numbers again. The ground was in the right place: at 960 × 540 the
    horizon is 24 px below the center, because the ray that grazes the ground sphere points 5.2° below the
    horizontal, and `tan(5.2°) × 270 ≈ 24`. The small sphere ends 156 px below the center (as on 10-26), and
    the ground starts right under it. The ground is almost flat green because its normal is nearly
    `(0, 1, 0)`, and `0.5 * (n + 1) = (0.5, 1, 0.5)`.
  - With the shrinking interval, a separate `t < closest` check is not needed: `Sphere::Hit` already rejects
    every `t` outside the interval.
- Interview story: "How do you test code whose result must not depend on order?" My first closest-hit loop
  kept the last hit instead of the nearest one: the "closest so far" variable was never updated. With the
  ground sphere first in the list, the image was pixel-identical to the correct one. With the ground second,
  30 144 pixels were wrong. A test with one fixed order could have passed, so the test tries both orders,
  and I check that it fails on the broken code.
- Next tiny step: M2 step 5, the first unit test, about today's bug. Add doctest via FetchContent (a test
  target, registered with `ctest`). Put two spheres on one ray and check that `Scene::Hit` returns the `t`
  of the nearer one in both list orders. Then bring back the bug for a minute (drop the line that shrinks
  `max`) and see the test fail. Optional 5-minute step first: a `DemoScene()` in core, so the two `main`s
  stop duplicating the scene (the CLI's comment still says "a single sphere").

## 2026-10-27 — M2: normals, `b = −2h`, `Sphere` and `Scene`

- Done:
  - M2 step 2 (RTiOW 6.1): the hit function returns the nearer root `t`, and the sphere is colored by its
    normal, `0.5 * (n + 1)` (RTiOW Image 4).
  - M2 step 3 (RTiOW 6.2): `a = SquaredLength()`, `h = d · oc`, the discriminant is `h² − ac`.
  - M2 step 4, first half (RTiOW 6.3, 6.5): `core/interval.h`, `core/hit_record.h`, `core/sphere.h/.cpp`
    (`Sphere::Hit` returns `std::optional<HitRecord>`: the nearer root inside the interval, else the farther
    one) and `core/scene.h` (`Scene { std::vector<Sphere> spheres; }`; `Scene::Hit` is only declared yet).
  - `RenderPixel(const Scene &, x, y, width, height)`. The app owns the scene (`AppState::scene`) and
    `AdvanceRender` passes it down; the CLI builds its own. Both start with the same one-sphere scene, and
    `RayColor` asks only the first sphere for now.
  - Checked: the refactor changes no pixel (0 of 518 400 differ from the 6.2 render). A sphere at z = +1 is
    gone (0 pixels; on 10-26 it was a full disc), and with the camera inside a sphere every pixel hits (the
    far root).
- Decided (the M2 design choice):
  - Scene objects are values: `std::vector<Sphere>`, not RTiOW's `virtual` + `shared_ptr`. The first book has
    one shape type, so up to M9 a "hittable" interface would have exactly one implementation. A `Sphere` is
    16 bytes, and the vector holds them back to back; the book's sphere is 24 bytes (the vtable pointer) plus
    one heap block per object.
  - A second shape type (Beyond) makes it `std::variant<Sphere, ...>` + `std::visit`. A whole triangle mesh
    can be one alternative, with its triangles (and later its own BVH) inside, behind the same `Hit`, like
    the two levels of GPU ray tracing APIs (a BLAS per mesh, a TLAS over them). The switch is mechanical: a
    `Shape` alias and one `visit` line in `Scene::Hit`.
  - `std::optional<HitRecord>` instead of `bool` + an out parameter: there is no half-filled record.
  - Names: the book's `world` / `hittable_list` is `Scene`. `hittable` has no counterpart (yet).
  - The scene is a parameter, not a global: the caller owns it, and `RenderPixel` gets a `const Scene &`, so
    it still has no hidden state (M8).
- Learned:
  - Where `b = −2h` comes from (RTiOW 6.2 does not say). The sphere equation is a square,
    `|t·d − oc|² = r²`: expanding it doubles the middle term (`−2t(d · oc)`), and the `2a` and `4ac` of the
    quadratic formula come from completing the square, which halves it again. Completing the square of
    `a·t² − 2h·t + c = 0` directly gives `t = (h ± √(h² − ac)) / a`, without any 2s and 4s. (The 3.x
    edition of the book called it `half_b`.)
  - Geometrically, with `|d| = 1`: `h` is the `t` of the point on the ray closest to the center,
    `m² = |oc|² − h²` is the squared distance of the center from the ray, and `h² − c = r² − m²` is the
    squared half chord `s²`. So `t = h ± s`: go to the closest point, then half a chord back or forth. The
    ray hits when it passes within `r` of the center; at the silhouette `m = r` (the 156 px of 10-26).
- Interview story: "Why values instead of `virtual` + `shared_ptr`?" The interface would have had one
  implementation for eight milestones, and every pointer costs something later. With values an index names
  an object (M6 picking), the threads read a vector of 16-byte spheres without touching a shared reference
  count (the book's `rec.mat = mat` copies a `shared_ptr` on every hit, M8), and the GPU gets the array in
  one copy (M9). If a second shape type arrives, a `std::variant` keeps the values.
- Next tiny step: `Scene::Hit` (RTiOW 6.5): loop over `spheres` and keep the closest hit by shrinking the
  interval's `max` to the last hit's `t` (the book's `closest_so_far`). `RayColor` calls `scene.Hit(...)`
  instead of asking `spheres[0]`. Then the ground sphere, `Sphere{Point3{0, -100.5f, -1}, 100}`, in one
  function that both `main`s call (e.g. `DemoScene()` in core). Expected: RTiOW Image 5. After that, 6.4's
  `front_face` (the comment in `HitRecord` already promises "always against the ray").

## 2026-10-26 — M2: the first sphere (a red disc)

- Done:
  - M2 step 1 (RTiOW 5.1–5.2, Listing 11): `IsSphereHit(sphere_center, radius, ray)` in the unnamed namespace
    in `render.cpp`, next to `RayColor`. `origin_to_center = sphere_center - ray.origin` (P − P → V),
    `a = d · d`, `b = −2 d · oc`, `c = oc · oc − r²`; it hits if `b² − 4ac ≥ 0`.
  - `RayColor` returns red when the ray hits the sphere at `Point3{0, 0, -1}` with radius 0.5. The viewport
    shows a red disc in the middle of the sky.
  - Not done, and not needed any more: the `constexpr` + `static_assert` idea from the last step. Step 2
    adds a `sqrt`, so the function stops being `constexpr` (Clang), and hit / miss / tangent / inside
    become unit tests in step 5.
  - By Claude: `-Wdouble-promotion` is in `umbra_warnings` now (root `CMakeLists.txt`; GCC and Clang only).
    In the whole project it flags exactly the two lines below in `render.cpp`, nothing else.
- Learned:
  - The disc can be checked with numbers, like the corner colors in M1. At 960 × 540 the viewport is 2 units
    high, so one unit at z = −1 is 270 px. The disc is 312 px wide and 312 px tall (x 324–635,
    y 114–425): round, centered, radius 156 px. Not 0.5 × 270 = 135 px: the rays that graze the sphere touch
    it in front of its center, so the silhouette on the viewport plane has the radius
    `tan(asin(r / d)) = 0.5 / sqrt(0.75) ≈ 0.577` units, and 0.577 × 270 ≈ 156 px.
  - The discriminant only says whether the *line* meets the sphere: not where, and not on which side of the
    camera. With the sphere at z = +1, behind the camera, the picture is the same (0 pixels differ;
    RTiOW 5.2: "This is not a feature!"). Step 2 fixes it: only a hit with `t > 0` counts.
  - `-2.0` is a `double` literal, so `b` and the discriminant are `double`, while `a` and `c` are `float`.
    The picture does not change, and `-Wall -Wextra -Wpedantic` say nothing; `-Wdouble-promotion` (GCC and
    Clang) points at both lines. It will matter in M9: GeForce cards run `double` at 1/32 to 1/64 speed, and
    one literal pulls a whole expression into `double`. (The radius `0.5` at the call is a `double` too,
    but it is converted to `float` once, exactly.)
- Next tiny step: first `float` literals in `IsSphereHit` and its call (`-2.0f`, `0.5f`; 2 minutes; the
  build is warning-free again when they are right). Then
  M2 step 2 (RTiOW 6.1): return the hit distance `t` as a `float` instead of a `bool` (−1 on a miss,
  otherwise `(-b - sqrt(discriminant)) / (2 * a)`, the nearer of the two roots; the `Is...` name will not
  fit any more). In `RayColor`, if `t > 0`: `n = (ray.At(t) - sphere_center).UnitVector()` (P − P → V), and
  the color is `0.5 * (n + 1)` per component. Before rendering, work out the expected colors at the center
  of the disc and at its top, left and right edges. Check: moved to z = +1, the sphere must disappear.

## 2026-10-26 — M1 done: camera and sky

- Done:
  - M1 step 5 (RTiOW 4.2, Listings 9–10): `RenderPixel` builds one ray per pixel from a hard-coded camera:
    the center at the origin, focal length 1, viewport height 2, and the viewport width from the image's
    real aspect ratio (the resolution comes from the *Render* panel, so there is no fixed 16:9 as in the
    book). Then `pixel_delta_u/v`, `pixel00_loc`, and `direction = pixel_center - camera_center`.
  - `RayColor(const Ray &)` in an unnamed namespace in `render.cpp`. First it returned the direction as a
    color, `0.5 * (UnitVector(direction) + 1)`, as a check. Now it returns the sky:
    `a = 0.5 * (unit.y + 1)`, then `(1 − a) * white + a * {0.5, 0.7, 1.0}`.
  - Cleanup: the dead gradient code and the unused `<cstdint>` are gone, the parameter names in `render.h`
    match the definition (`image_width`, `image_height`), and the fixed camera values are `constexpr`.
  - M1 is finished: `kMilestone` is "M1 - first light", `docs/gallery/m1-first-light.png` is in the README,
    ▶ moved to M2. (The `Texture` side quest was done on 10-15; its boxes are ticked now.)
- Learned:
  - Listing 9 is a complete ray tracer, and in Umbra most of it already existed elsewhere. The *Image* block
    is the app (the resolution); the two loops and the `std::clog` progress are `AdvanceRender` (rows within
    a 12 ms budget, the progress bar); `write_color` is `ToPixel`. Only the camera block and the loop body
    went into `RenderPixel`, so one file changed.
  - The viewport is in world units, the image in pixels. Taking the viewport width from the real
    `width / height` keeps the pixels square at any resolution.
  - The image's y grows downward, the world's y upward: hence `viewport_v = (0, −h, 0)`. In Umbra `y = 0` is
    the top row, like the book's `j`, so the formulas carry over unchanged.
  - `Point3` paid off: the listing type-checks as written. `viewport_upper_left` is P − V − V − V → P,
    `pixel00_loc` is P + V → P, and the direction is P − P → V.
  - Listing 9's all-black image tests nothing: it looks the same when every ray is wrong. The direction as a
    color can be checked with numbers. At 16:9 the unit direction to a corner is about (±0.78, ±0.44, −0.44),
    so the corners are green at the top left (0.11, 0.72, 0.28), gold at the top right (0.89, 0.72, 0.28),
    red at the bottom right (0.89, 0.28, 0.28) and dark teal at the bottom left (0.11, 0.28, 0.28), and the
    center is olive (0.5, 0.5, 0). The screenshot matched, which checked the direction, the vertical
    orientation and the aspect ratio at once.
  - That picture also relied on 10-25's unclamped `Color` arithmetic. The direction's components are
    negative on the left, at the bottom, and (z) everywhere; a clamping `*` would have made them 0 before
    the `+ 0.5`.
  - Recomputing the camera for every pixel costs nothing visible yet: 960 × 540 in 0.006 s in the GUI.
    M3's camera class computes it once anyway.
- Interview story: "How do you debug a renderer when all you get is colors?" Make the value you want to
  check visible as a color, and pick one whose picture you can predict with numbers. Listing 9 renders
  black, which is also what wrong rays render. Mapping the ray direction to RGB gave four corner colors I
  could compute by hand, and one look checked the direction, the orientation and the aspect ratio. M2 does
  the same with surface normals.
- Next tiny step: M2 step 1 (RTiOW 5.1). In `render.cpp`, next to `RayColor`, write
  `bool HitSphere(const Point3 &center, float radius, const Ray &ray)`: `oc = center - ray.origin`
  (P − P → V), `a = ray.direction.Dot(ray.direction)`, `b = -2 * ray.direction.Dot(oc)`,
  `c = oc.Dot(oc) - radius * radius`; it hits if `b * b - 4 * a * c >= 0`. In `RayColor`, return red when
  the ray hits the sphere at `Point3{0, 0, -1}` with radius 0.5. Expected: a red disc in the middle of the
  sky. (There is no `sqrt` in it, so it can be `constexpr` with two `static_assert`s: a ray straight ahead
  hits, one straight up misses.)

## 2026-10-25 — M1: `Color` arithmetic

- Done:
  - `src/core/color.h`: `Color` got `operator+`, `float * Color` and `Color * float` as `constexpr` hidden
    friends. No clamping inside them: only `ToPixel` clamps.
  - `static_assert`s for addition, scaling, the commutativity of scaling, and scale-then-add. The results
    are above 1 on every component, so a `clamp` sneaking back in fails the build.
- Learned:
  - My first version clamped to [0, 1] in `+` and `*`. The sky would still have looked right, because all
    its values are in [0, 1]. The bug would have shown up later: the sum of 100 samples of 0.5 saturates at
    1.0, so the average is 0.010 instead of 0.5 (M3's accumulation buffer); `4 * {0.5, 0.25, 0.125}` gave
    `{1, 1, 0.5}` instead of `{2, 1, 0.5}` (lights brighter than white, M4); and `0.5 * (a + b)` differed
    from `0.5 * a + 0.5 * b` (for `a = b = 0.75`: 0.5 vs 0.75), so even the algebra broke.
  - Light is linear and unbounded; [0, 1] is only what the display can show. The book's `vec3` colors are not
    clamped either, the clamping happens once, when the color is written out. In Umbra that place is
    `ToPixel`. The ROADMAP's old wording of step 2 ("colors in [0, 1]") invited this mistake; it is reworded.
  - A bug that no picture shows yet needs a test that looks for it: values outside [0, 1] in the asserts.
    Checked by mutation: a `clamp` added back on `r`, `g` or `b`, in either operator, fails the build.
  - Test values that are binary fractions (0.75, 0.875, 1.5) make the float arithmetic exact, so the exact
    `==` is reliable.
- Interview story: "Where do you clamp a color in a renderer?" Once, in the final conversion to bytes.
  Before that, colors are linear floats that may exceed 1: a light is brighter than white, and a sum of
  samples is N times a pixel until it is divided. A `clamp` inside `operator+` looks harmless and the first
  picture is still right, but averaging 100 samples of 0.5 then gives 0.01. An operation that "fixes" its
  result destroys information the next operation needs.
- Next tiny step: M1 step 5, the camera. Read RTiOW 4.2 (the viewport figure) first. Then, in `RenderPixel`,
  with a hard-coded camera (`Point3{}` as the center, focal length 1, viewport height 2, the width from the
  aspect ratio; M3 moves it into a class), build the ray for each pixel: its `direction` is
  `pixel_center - camera_center`. Look at it before the sky: color the pixel by
  `0.5 * (UnitVector(direction) + 1)` per component. The sky gradient is the next session.

## 2026-10-25 — M1: `Point3` and `Ray`

- Done:
  - Decided: `Point3` is its own type, not `using Point3 = Vector3;` like the book. The compiler now rejects
    nonsense (`P + P`, `P * float`, `V − P`, `Dot` on a point), and the whole of RTiOW needs only three mixed
    operations. Later it matters for transforms (M6–M7): a translation moves a point but not a vector.
  - `src/core/point3.h`: `umbra::Point3`, an aggregate of three `float`s with `constexpr` hidden friends
    `P − P → V`, `P + V → P`, `P − V → P`, and a defaulted exact `==`. No `NearlyEqual` and no epsilon yet,
    nothing needs them.
  - What must *not* compile is pinned too: `!CanAdd<Point3, Point3>`, `!CanMultiply<Point3, float>`,
    `!CanSubtract<Vector3, Point3>`, and no implicit conversion between `Point3` and `Vector3` in either
    direction. The concepts live in `umbra::detail`.
  - `src/core/ray.h`: `Ray { Point3 origin; Vector3 direction; }`, an aggregate, with `constexpr At(t)`
    (`origin + t * direction`) and a defaulted `==`. The direction is documented as not necessarily a
    unit vector. `static_assert`s at `t` = 0, 1, 2 and −0.5. `render.cpp` includes `ray.h`, so the whole
    chain (`ray.h` → `point3.h` → `vector3.h`) is compiled and checked.
  - M1 step 4 is done.
- Learned:
  - A `requires` expression outside a template is not `false` when its expression is ill-formed, it is a
    hard error. So "must not compile" needs a concept (a template):
    `template <typename L, typename R> concept CanAdd = requires(L l, R r) { (l + r); };`, then
    `static_assert(!CanAdd<Point3, Point3>)`. The parentheses keep `clang-format` from rewriting `l * r` as
    `l *r`. Checked the check: adding a `Point + Point` operator by mistake makes the build fail.
  - Mutation testing again. Every mutation of `P + V` and `P − V` survived the first `Point3` asserts, and
    `{1, 2, 3} − {4, 5, 6} = {-3, -3, -3}` would hide a mixed-up component. For `Ray`, `t = 0` and `t = 1`
    are the two values where almost any function of `t` is right (0 → 0, 1 → 1): `t * t`, `abs(t)` and a
    clamp to [0, 1] all survived until `t = 2` and `t = -0.5` were added. Rule of thumb: test with values
    that differ from each other and are not the identity values 0 and 1.
  - The open question from 10-13 (the midpoint of two points without `Point + Point`) has a plain answer:
    `a + (b - a) / 2`.
  - The direction of a ray is not normalized, so `t` is a distance only if `|direction| = 1`. M2's quadratic
    uses `Dot(d, d)` instead of assuming 1.
- Interview story: "How do you make invalid operations unrepresentable, and how do you test it?" `Point3`
  and `Vector3` hold the same three floats, but their operator sets differ: `P − P → V`, `P + V → P`,
  `P − V → P`, and nothing else. `P + P` is simply not declared, so it does not compile. A test that
  something does *not* compile is `static_assert(!CanAdd<Point3, Point3>)` on a concept (inside a template an
  ill-formed `requires` is `false`, not an error).
- Next tiny step: M1 step 5, the sky. The gradient is `(1 − a) * white + a * blue`, and `Color` has no
  arithmetic yet. So first give `Color` `operator+` and `float * Color` as `constexpr` hidden friends,
  each with a `static_assert` (values that are not 0 or 1), in the style of `Vector3`. Then read RTiOW 4.2
  (the viewport); the camera itself is the next session.

## 2026-10-25 — M1: `Vector3`

- Done (10-24 and 10-25):
  - `static_assert`s for `ToPixel` in `core/color.h` (the step that was open since 10-13).
  - `src/core/vector3.h`: `umbra::Vector3`, three `float`s, a plain aggregate. `constexpr` hidden-friend
    operators `+`, `-`, unary `-`, `float * V`, `V * float`, `V / float`, and a defaulted, exact `==`.
    Members: `SquaredLength()`, `Dot()`, `Cross()`, `Length()` and `UnitVector()` (the last two are not
    `constexpr`, see the interview story). `NearlyEqual(a, b, epsilon)` is a separate function with
    `kVector3Epsilon = 1e-6f`. `render.cpp` includes the header, so the compiler really checks its
    `static_assert`s (a header nobody includes is never compiled).
  - `static_assert(std::is_aggregate_v<T>)` and `std::is_trivially_copyable_v<T>` for `Vector3`, `Color` and
    `Pixel`. `Pixel`'s `==` is defaulted inside the struct now, so it cannot forget a member.
  - Naming: constants are `kCamelCase` (Google style) at namespace scope and `snake_case` inside
    functions. `.clang-tidy` and `CLAUDE.md` say so.
  - Decided: my own `Vector3`, not raylib's (the core cannot see `raylib.h`, raylib's is not `constexpr`,
    and M9 needs code that also runs on CUDA). `float`, not a template: `Color` is `float`, and so is M9.
  - Decided: no tags, mixins or CRTP (the 10-13 design notes). They would automate the easy part
    (`T op T → T`); the interesting operations mix types. One `Vector3` for everything, like the book.
    Whether positions get their own `Point3` is decided at `Ray` (M1 step 4).
- Learned:
  - An aggregate has no user-declared constructor (since C++20 even `= default` counts), no private
    members and no virtual functions. `Vector3{1, 2, 3}` copies the values into the members in order, no
    constructor runs; missing values come from the `{}` member initializers. My first version had
    constructors that left the members uninitialized: `Vector3 v;` held garbage, and only `Vector3{}` was
    zero (value-initialization). Without constructors `Vector3 v;` is zero too, and the type follows the
    rule of zero: trivially copyable, `sizeof` is three floats.
  - `==` with a tolerance is not an equality. It is not transitive (0, 6e-7 and 1.2e-6: `a == b`,
    `b == c`, but `a != c`), and a fixed `1e-6` stops meaning "close" above 16, where neighbouring floats
    are further apart than that. So `==` is exact, and the tolerance is an explicit `NearlyEqual` at the
    call site. Whether it needs a relative tolerance waits for the M2 tests.
  - A `static_assert` is only worth something if it can fail. Mutation testing by hand: break one term in
    the code and see whether any assert notices. The axis-vector tests missed all of `Dot`, the `y` of
    unary minus and two of the six terms of `Cross`. `{1, 2, 3} × {4, 5, 6} = {-3, 6, -3}` uses all six
    products (12, 15, 12, 6, 5, 8), and `Cross(a, b) · a == 0` checks the geometry.
  - `Dot(a, b) = |a||b| cos θ`: how much two vectors point the same way (0 = perpendicular), and
    `Length(v) = sqrt(Dot(v, v))`. `Cross(a, b)` is a vector perpendicular to both, as long as the area of
    their parallelogram. Needed for: ray–sphere (M2, dots), shading and reflection (M4, dots), the camera
    basis (M5, crosses).
  - `UnitVector()` of the zero vector is NaN (0/0). It is a documented precondition (`@pre`), as in the book.
- Interview story: "Why does your `static_assert` pass on GCC and fail on Clang?" `Length()` was
  `constexpr` and had `static_assert(Vector3{3, 4, 0}.Length() == 5.0f)`. GCC accepted it; Clang said
  "static assertion expression is not an integral constant expression" and pointed at `__builtin_sqrtf`
  inside `<cmath>`. `std::sqrt` is `constexpr` only from C++26. GCC folds it as a builtin anyway (an
  extension); the standard does not promise it. (MSVC: not tried yet.) Fix: `Length()` and `UnitVector()`
  lost `constexpr`, their asserts became `SquaredLength()` asserts, and the real checks wait for the unit
  tests (M2 step 5). I only saw it because the header was compiled with a second compiler (clang-tidy
  uses Clang as well).
- Next tiny step: M1 step 4, `src/core/ray.h`. First decide `Point3`: `using Point3 = Vector3;` like the
  book, or its own type with three mixed operators (`P − P → V`, `P + V → P`, `P − V → P`; about 15
  lines). Write the decision here. Then `struct Ray { Point3 origin; Vector3 direction; }` with a
  `constexpr` `At(float t)` (`origin + t * direction`), one `static_assert`, and an include from
  `render.cpp`.

## 2026-10-15 — M1 side quest: RAII `Texture` wrapper

- Done (10-14 and 10-15):
  - `src/app/texture.h/.cpp`: `Texture` owns the raylib `Texture2D`. The destructor unloads it, copying is
    deleted, moving transfers ownership (`std::exchange` leaves an empty handle behind). The destructor
    and the move assignment share a private `Unload()`: if valid, unload, then reset the handle to `{}`,
    so a second call does nothing. Besides that it has only `Upload()` and one accessor,
    `const ::Texture2D &Get() const`. No rlImGui include: the viewport calls
    `rlImGuiImageSizeV(&app.texture.Get(), ...)` itself.
  - `Upload(std::span<const Pixel>)` always uploads and `assert`s its precondition: the texture is RGBA8
    and the span holds width × height pixels (`UpdateTexture()` reads that much, whatever the buffer
    holds). `Framebuffer::Pixels()` returns the span; `Data()` stays for C APIs (the PNG export).
  - `AppState::texture` is a `Texture` now; the manual `UnloadTexture()` calls are gone. The dirty flag
    stays in `AppState` (`texture_is_dirty`): `UploadTexture()` in `render_job.cpp` decides *when* to
    upload, the texture only knows *how*.
  - `Run()` in `main.cpp`: `AppState` lives in an inner block that ends before `rlImGuiShutdown()` and
    `CloseWindow()`. A workaround, see *Next tiny step*.
- Learned:
  - `DrawViewport(const AppState &)` makes `app.texture` const, and a const object can only call `const`
    members. Inside a `const` member, `&texture_` is a `const ::Texture2D *`, so a `GetAddress() const`
    returning `::Texture2D *` does not compile.
  - Don't give the resource owner one method per library that uses it (`Texture::rlImGuiImageSizeV()`).
    Lend the raw handle instead, like `unique_ptr::get()` or `vector::data()`. One `const &` accessor is
    enough: every rlImGui function takes `const Texture *`, raylib takes `Texture2D` by value (except
    `GenTextureMipmaps`). A non-const `::Texture2D &Get()` would be a hole: `app.texture.Get() = {}` leaks.
    The const is shallow, as with `const unique_ptr<T>`: it protects the handle, not the GPU object.
  - `std::move` only casts. On a plain C struct like `Texture2D` the "move" is a copy, so the source still
    holds the handle. A move must leave the source in a state where its destructor does nothing
    (`std::exchange(other.texture_, {})`; id 0 makes `IsTextureValid()` false).
  - `app.texture = Texture{...}` needs no `std::move`: a temporary is already an rvalue, so overload
    resolution picks the move assignment (by rule, not by optimization). `std::move` is for named objects.
  - Naming: in the standard library `unique_ptr::release()` gives up ownership *without* freeing, and
    `reset()` frees. So my "free it" function is `Unload()`, not `Release()`.
  - `std::span` (C++20) carries the size with the pointer, so the callee can check it. A bare
    `const Pixel *` can't. The asserts run only in the `debug` preset (`-DNDEBUG` in release).
  - RAII ties cleanup to scope: locals die in reverse order of construction, at the closing brace. The GL
    context is a resource too, but `InitWindow()`/`CloseWindow()` are manual calls, so `app` was destroyed
    after the window. gdb showed `main` → `~AppState()` → `UnloadTexture` → `rlUnloadTexture` → SIGSEGV
    (exit code 139).
  - `layout_path` must outlive `rlImGuiShutdown()` (ImGui saves `layout.ini` there), so it stays outside
    the inner block.
- Interview story: "Explain the rule of 0/3/5." My first move constructor did
  `texture_ = std::move(other.texture_)`. It compiled, but the temporary kept the handle and its
  destructor deleted the GPU texture the app was still using: garbage in the viewport. After the fix,
  the app segfaulted on exit, because RAII had moved the unload from "before `CloseWindow()`" to "end of
  `Run()`". Both bugs have the same cause: with RAII the *when* of cleanup is decided by scopes, not by
  the order of calls, so every resource has to be an object, the window as well.
- Possible cleanup (optional): viewport_panel.cpp still has the old call commented out.
- Next tiny step: back to the core. Add `static_assert`s for `ToPixel` in `core/color.h` (black -> 0,
  white -> 255, 0.5 -> 128, `{2, -1, 0.5}` -> `{255, 0, 128}`), the step from 10-13 that is still open.
  Optional side step: make the window a resource (a small class that calls `InitWindow` + `rlImGuiSetup`
  and their shutdown calls, declared before `app`), then the inner block can go.

## 2026-10-13 — Plumbing: main.cpp split, one CMakeLists.txt per target

- Done (by Claude; nothing changes on screen: the before/after screenshots differ only in the FPS and
  speed numbers):
  - `src/app/main.cpp`: 737 → 160 lines, start-up and the main loop only. The rest is in `app_state.h`,
    `render_job.cpp`, `ui_scale.cpp`, `command_line.cpp` and `panels/` (one file per window).
  - New target `umbra_ui` in `src/ui/`: `theme.cpp` (moved from `src/app/`) and `widgets.cpp` (the
    property grid). It knows nothing about the app.
  - One `CMakeLists.txt` per target directory; the root one has a table of what links what. The
    executables are still in `build/<preset>/`.
  - `umbra_io` links raylib `PRIVATE`; `umbra-cli` lists raylib itself.
  - `CMAKE_CXX_SCAN_FOR_MODULES OFF`: no modules here, and the scan flags GCC 14 gets (`-fmodules-ts`,
    ...) ended up in `compile_commands.json` and broke clang-tidy and clangd.
- Learned:
  - CMake vs. Bazel: `PUBLIC` ≈ `deps`, `PRIVATE` ≈ `implementation_deps`. A PRIVATE dependency of a static
    library still reaches the executable's link line; what PRIVATE stops is the headers and compile
    flags. Nothing checks that a target includes only what it lists (no layering_check): `src/` is the
    include root for everyone.
  - raylib's color macros are a trap inside `namespace umbra`: `BLACK` expands to `Color{ 0, 0, 0, 255 }`,
    which means `umbra::Color` there as soon as `core/color.h` is included ("too many initializers").
- Interview story: "How do you keep a build's dependency graph honest?" `umbra-cli` called raylib
  (`SetTraceLogLevel`) without listing it, and compiled only because `umbra_io` passed raylib on
  (`PUBLIC`). With `PRIVATE` it failed right away (`raylib.h: No such file or directory`), at the line
  that had the hidden dependency.
- Design notes from the chat (not decided yet): a `Vector3Base<T, Tag, Features...>` with mixins like
  `Additive` (T + T → T) would allow `Position + Position`, exactly what the tags should forbid. The
  interesting operations mix types: Point − Point → Vector, Point + Vector → Point. Direction and
  Translation are both vectors (unit length is an invariant, not a separate algebra). So: hand-written
  `Point3` and `Vector3` (later `Normal3`, which transforms differently); first the table of allowed
  operations as `static_assert`s, plus a list of what must not compile; a CRTP base (like pbrt-v4's
  `Tuple3`) only when the duplication hurts. Open question: the midpoint of two points, if
  `Point + Point` is not allowed?
- Next tiny step: add `static_assert`s for `ToPixel` in `core/color.h` (black -> 0, white -> 255,
  0.5 -> 128, `{2, -1, 0.5}` -> `{255, 0, 128}`). Then read RTiOW 3.1 and write the bare vector type(s)
  with `operator+` and `operator-`, one `static_assert` each.

## 2026-10-13 — M1: gradient, colors as floats

- Done (two sessions, 10-12 and 10-13):
  - M1 step 1: `RenderPixel` draws the RTiOW chapter 2 gradient (black, red, green and yellow corners).
  - M1 step 2: `core/color.h` with `Color` (three floats, `r`/`g`/`b`) and `constexpr ToPixel()`, the one
    place where a color becomes bytes. It clamps to [0, 1] and rounds to the nearest byte.
  - Naming convention: `CamelCase` types and functions, `snake_case` variables; `.clang-tidy` checks it
    (`.clang-format` cannot). Headers renamed from `.hpp` to `.h`. `umbra-cli` rejects sizes below 2.
  - GUI: the UI scale follows the system or *View ▸ UI scale*; settings are saved in `~/.config/umbra/`.
- Learned:
  - `x / (width - 1)` with two `int`s is integer division: 0 everywhere except the last column.
  - Float to byte, three ways. `c * 255` truncates, so only exactly 1.0 gives 255. `c * 255.999` (the
    book) gives every byte an equal slice of [0, 1]. `c * 255 + 0.5` rounds to the nearest byte: byte `k`
    means `k / 255`, so the error is at most half a step and pixel -> color -> pixel gives the same byte.
    I chose rounding. With `+ 0.5f` instead of `std::round` it stays `constexpr` everywhere.
  - `clang-format` only does layout; naming rules are `clang-tidy`'s `readability-identifier-naming`.
- Interview story: "Why did my `constexpr` function fail to link?" `ToPixel` was declared in `color.h` and
  defined in `color.cpp`. GCC warned `inline function ... used but never defined`, then the linker said
  `undefined reference`. The reason: `constexpr` functions are implicitly `inline`, and an inline function
  must be defined in every translation unit that uses it. In `color.cpp` nothing called it, so the
  compiler emitted no symbol at all. Fix: the body goes into the header. (Same rule for templates, so all
  of `Vector3` will live in a header.)
- Open design question: a tagged `Vector3<Tag>` so that `Position` and `Direction` are different types.
  Distinct types pay off when each has its own set of operations, so the real work is deciding the
  operators between tags (`Position − Position` is a `Direction`; `Position + Direction` is a
  `Position`). `Color` stays a separate struct: it shares almost no operations with the geometric types
  and wants `r`/`g`/`b`. pbrt does the same (`Point3` / `Vector3` / `Normal3`, RGB separate).
- Next tiny step: add `static_assert`s for `ToPixel` in `core/color.h` (black -> 0, white -> 255,
  0.5 -> 128, `{2, -1, 0.5}` -> `{255, 0, 128}`). Then read RTiOW 3.1 and write the bare `Vector3`
  struct with `operator+` and `operator-`, with one `static_assert` each.

## 2026-10-11 — M0: skeleton

- Done: project skeleton set up by Claude: CMake + presets, raylib 6.0 / ImGui docking / rlImGui,
  GUI with *Viewport* and *Render* panels, progressive row rendering, *Save PNG*, `umbra-cli`,
  `--screenshot`. Builds with GCC 13 and 14 (Ubuntu 24.04). Not yet tried with MSVC.
- Learned: raylib is not a ray tracer. It is a small C library for windows, input and drawing,
  named after its author's nickname, Ray.
- Interview story (from the Mandelbrot project, before this one): the CPU and CUDA renderers were
  byte-compared by `make test`. Building the CPU version with and without `-ffp-contract=off` changes
  0.016 % of the pixels in the default view and 0.12 % in a zoomed view. The reason: with
  `-march=native`, GCC fuses `a*b + c` into FMA instructions by default in C++, and `nvcc` does the
  same (`--fmad=true`). FMA rounds once instead of twice, and in a chaotic iteration a one-ulp
  difference can change the escape count. So whether the byte-for-byte test passes depends on
  compiler choices, not on the source. Lesson: compare floating-point images with a tolerance.
- Next tiny step: build and run `umbra` in WSL, toggle *Slow motion*, then `git init` and make the
  first commit.
