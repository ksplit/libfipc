static void* XOR_checksum ( void* pkt, const uint64_t num_words )
{
	if ( num_words == 0 )
		return pkt;
	
	uint64_t* data = (uint64_t*) pkt;

	int i;
	for ( i = 1; i < num_words; ++i )
		data[0] ^= data[i];
	
	return pkt;
}

static void* SUM_checksum ( void* pkt, const uint64_t num_words )
{
	if ( num_words == 0 )
		return pkt;
	
	uint64_t* data = (uint64_t*) pkt;

	int i;
	for ( i = 1; i < num_words; ++i )
		data[0] += data[i];
	
	return pkt;
}

static void* MUL_checksum ( void* pkt, const uint64_t num_words )
{
	if ( num_words == 0 )
		return pkt;
	
	uint64_t* data = (uint64_t*) pkt;

	uint64_t i;
	for ( i = 1; i < num_words; ++i )
		data[0] *= data[i];
	
	return pkt;
}

static void* CPU_intense ( void* pkt, const uint64_t num_words )
{
	if ( num_words == 0 )
		return pkt;
	
	uint64_t* data = (uint64_t*) pkt;

	uint64_t i;
	for ( i = 1; i < CPU_OPERATIONS; ++i )
		data[0] *= (data[0] + i);
	
	return pkt;
}

static void* Fletcher_checksum ( void* pkt, const uint64_t num_words )
{
	if ( num_words == 0 )
		return pkt;
	
	uint32_t* data = (uint32_t*) pkt;

	uint64_t sum1 = 0;
	uint64_t sum2 = 0;

	uint64_t i;
	for ( i = 0; i < num_words; ++i )
	{
		sum1 = ( sum1 + data[i] ) % 4294967296;
		sum2 = ( sum2 + sum1    ) % 4294967296;
	}

	((uint64_t*)pkt)[0] = ( sum2 << 32 ) | sum1;

	return pkt;
}

// CITE: https://locklessinc.com/articles/tcp_checksum/
static void* TCPIP_checksum ( void* pkt, const uint64_t num_words )
{
	if ( num_words == 0 )
		return pkt;
	
	unsigned size = num_words*8;
	unsigned long long sum = 0;
	const unsigned long long *b = (unsigned long long *) pkt;

	unsigned t1, t2;
	unsigned short t3, t4;

	/* Main loop - 8 bytes at a time */
	while (size >= sizeof(unsigned long long))
	{
		unsigned long long s = *b++;
		sum += s;
		if (sum < s) sum++;
		size -= 8;
	}

	/* Handle tail less than 8-bytes long */
	const char* buf = (const char *) b;
	if (size & 4)
	{
		unsigned s = *(unsigned *)buf;
		sum += s;
		if (sum < s) sum++;
		buf += 4;
	}

	if (size & 2)
	{
		unsigned short s = *(unsigned short *) buf;
		sum += s;
		if (sum < s) sum++;
		buf += 2;
	}

	if (size)
	{
		unsigned char s = *(unsigned char *) buf;
		sum += s;
		if (sum < s) sum++;
	}

	/* Fold down to 16 bits */
	t1 = sum;
	t2 = sum >> 32;
	t1 += t2;
	if (t1 < t2) t1++;
	t3 = t1;
	t4 = t1 >> 16;
	t3 += t4;
	if (t3 < t4) t3++;

	unsigned short* data = (unsigned short*) pkt;
	data[0] = ~t3;

	return pkt;
}
