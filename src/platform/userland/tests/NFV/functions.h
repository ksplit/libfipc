static void* XOR_checksum ( void* pkt, const uint64_t num_words )
{
	uint64_t* data = (uint64_t*) pkt;

	if ( num_words == 0 )
		return 0;

	int i;
	for ( i = 1; i < num_words; ++i )
		data[0] ^= data[i];
	
	return pkt;
}

static void* SUM_checksum ( void* pkt, const uint64_t num_words )
{
	uint64_t* data = (uint64_t*) pkt;
	
	if ( num_words == 0 )
		return 0;

	int i;
	for ( i = 1; i < num_words; ++i )
		data[0] += data[i];
	
	return pkt;
}

static void* MUL_checksum ( void* pkt, const uint64_t num_words )
{
	uint64_t* data = (uint64_t*) pkt;

	if ( num_words == 0 )
		return 0;

	uint64_t i;
	for ( i = 1; i < num_words; ++i )
		data[0] *= data[i];
	
	return pkt;
}

static void* Fletcher_checksum ( void* pkt, const uint64_t num_words )
{
	uint32_t* data = (uint32_t*) pkt;

	uint64_t sum1 = 0;
	uint64_t sum2 = 0;

	uint64_t i;
	for ( i = 0; i < num_words*2; ++i )
	{
		sum1 = ( sum1 + data[i] ) % 4294967296;
		sum2 = ( sum2 + sum1    ) % 4294967296;
	}

	((uint64_t*)pkt)[0] = ( sum2 << 32 ) | sum1;

	return pkt;
}
