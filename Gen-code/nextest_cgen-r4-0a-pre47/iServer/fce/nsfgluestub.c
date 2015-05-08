//This file defines empty functions to pass the linking stage
#include <inttypes.h>

void
nsfGlueInit(void)
{
	return;
}

int
nsfGlueShutdown(void)
{
	return 0;
}

int
nsfGlueReconfig(void)
{
	return 0;
}

int
nsfGlueOpenResource(	uint32_t	bundleId,
						uint32_t	dstIpAddress,
						uint16_t	dstPort,
						uint32_t	ingressPoolId,
						uint32_t	egressPoolId,
						char *		protocol,
						uint32_t	peerResourceId,
						uint32_t *	returnedIpAddress,
						uint16_t *	returnedPort,
						uint32_t *	returnedResourceId,
						int			allocateCreate)

{
	return 0;
}

int
nsfGlueModifyResource(	uint32_t	bundleId,
			uint32_t	resourceId,
			uint32_t	newDstIpAddress,
			uint16_t	newDstPort,
			uint32_t	newIngressPoolId,
			uint32_t	newEgressPoolId,
			uint32_t	peerResourceId,
			uint32_t *	returnedIpAddress,
			uint16_t *	returnedPort,
			int		allocateCreate)
{
	return 0;
}

int
nsfGlueCloseResource(uint32_t resourceId)
{
	return 0;
}

int
nsfGlueCloseBundle(uint32_t bundleId)
{
	return 0;
}

uint32_t
newHoleId(void)
{
	return 1;
}

