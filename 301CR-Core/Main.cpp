#include "Logger.h"


#ifdef BUILD_DEBUG
int main(char** argv, int argc)
{
	LOG_ERROR("Dog %s", "HELOO");
	return 0;
}
#else
int main(char** argv, int argc)
{
	LOG_ERROR("Dog %s", "HELOO");
	return 0;
}
#endif