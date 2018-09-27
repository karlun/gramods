
#include <gmGraphics/FreeImageInitializer.hh>

#ifdef gramod_ENABLE_FreeImage

BEGIN_NAMESPACE_GMGRAPHICS;

FreFreeImageInitializer::FreeImageInitializer() {
  	FreeImage_Initialise();
}

FreeImageInitializer::~FreeImageInitializer() {
  	FreeImage_DeInitialise();
}

END_NAMESPACE_GMGRAPHICS;

#endif
