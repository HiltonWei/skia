/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkRasterPipeline.h"

SK_RASTER_STAGE(load) {
    auto ptr = (const float*)ctx + x;
    switch(tail&3) {
        case 0: a = Sk4f{ptr[3]};
        case 3: b = Sk4f{ptr[2]};
        case 2: g = Sk4f{ptr[1]};
        case 1: r = Sk4f{ptr[0]};
    }
}

SK_RASTER_STAGE(square) {
    r *= r;
    g *= g;
    b *= b;
    a *= a;
}

SK_RASTER_STAGE(store) {
    auto ptr = (float*)ctx + x;
    switch (tail&3) {
        case 0: ptr[3] = a[0];
        case 3: ptr[2] = b[0];
        case 2: ptr[1] = g[0];
        case 1: ptr[0] = r[0];
    }
}

DEF_TEST(SkRasterPipeline, r) {
    // We'll build up and run a simple pipeline that exercises the salient
    // mechanics of SkRasterPipeline:
    //    - context pointers                           (load,store)
    //    - stages sensitive to the number of pixels   (load,store)
    //    - stages insensitive to the number of pixels (square)
    //
    // This pipeline loads up some values, squares them, then writes them back to memory.

    const float src_vals[] = { 1,2,3,4,5 };
    float       dst_vals[] = { 0,0,0,0,0 };

    SkRasterPipeline p;
    p.append<load>(src_vals);
    p.append<square>();
    p.append<store>(dst_vals);

    p.run(5);

    REPORTER_ASSERT(r, dst_vals[0] ==  1);
    REPORTER_ASSERT(r, dst_vals[1] ==  4);
    REPORTER_ASSERT(r, dst_vals[2] ==  9);
    REPORTER_ASSERT(r, dst_vals[3] == 16);
    REPORTER_ASSERT(r, dst_vals[4] == 25);
}

DEF_TEST(SkRasterPipeline_empty, r) {
    // No asserts... just a test that this is safe to run.
    SkRasterPipeline p;
    p.run(20);
}

DEF_TEST(SkRasterPipeline_nonsense, r) {
    // No asserts... just a test that this is safe to run and terminates.
    // square() always calls st->next(); this makes sure we've always got something there to call.
    SkRasterPipeline p;
    p.append<square>();
    p.run(20);
}
