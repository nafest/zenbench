### TODO
* supply means to avoid compiler optimizations
* the std::sort <-> qsort example shows that we need an initialization phase for every iteration
  (call setup/teardown for every iteration? call it implicitely in ctxt.Running()?)
    ctxt.BeginExclude(); -> stop time measurement
    ctxt.EndExclude(); -> continue time measurement