class SAnimationContext
{
  const SControllerDef *controllerDef;
  CTagState state;
  DynArray<CTagState,int,NArray::SmallDynStorage<NAlloc::AllocCompatible<NAlloc::ModuleAlloc> > > subStates;
  CMTRand_int32 randGenerator;
};
