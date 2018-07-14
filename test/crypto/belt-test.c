/*
*******************************************************************************
\file belt-test.c
\brief Tests for STB 34.101.31 (belt)
\project bee2/test
\author (C) Sergey Agievich [agievich@{bsu.by|gmail.com}]
\created 2012.06.20
\version 2018.06.29
\license This program is released under the GNU General Public License 
version 3. See Copyright Notices in bee2/info.h.
*******************************************************************************
*/

#include <bee2/core/mem.h>
#include <bee2/core/hex.h>
#include <bee2/core/u32.h>
#include <bee2/core/util.h>
#include <bee2/crypto/belt.h>

/*
*******************************************************************************
Самотестирование

-#	Выполняются тесты из приложения A к СТБ 34.101.31 и из приложения Б
	к СТБ 34.101.47.
-#	Номера тестов соответствуют номерам таблиц приложений.
-#	Дополнительно выполняется тест на основе задачи 
	http://apmi.bsu.by/resources/tasks#60: найти 128-битовые блоки 
	X_0,..., X_127 такие, что
		X_0 ^ ... ^ X_127 ^ Belt_0(X_0) ^ ... ^ Belt_0(X_127) = 0.
*******************************************************************************
*/

static const u32 _zerosum[128] = {
	15014,124106,166335,206478,313245,366839,455597,502723,535141,625112,
	659461,752253,801048,897899,943850,1041695,1101266,1170856,1217537,
	1248520,1366084,1421171,1448429,1514215,1573855,1701341,1738016,1781705,
	1837300,1948449,1999650,2089289,2117830,2175758,2249930,2358928,2404262,
	2447467,2552783,2556713,2678348,2705770,2808011,2827994,2948039,2995213,
	3029188,3096649,3170243,3230306,3285991,3350691,3457162,3500592,3539783,
	3636611,3735543,3752463,3814136,3875630,3935109,4002291,4088401,4129247,
	4257830,4266427,4352389,4397389,4470348,4531932,4598961,4691323,4747531,
	4839756,4900773,4958368,5021928,5099836,5164752,5214964,5269476,5356247,
	5391667,5496861,5561223,5601750,5700311,5761736,5812345,5856838,5956987,
	5966502,6059392,6104328,6193021,6233226,6311341,6369016,6475468,6540894,
	6598453,6666092,6711620,6804478,6834201,6932158,6971325,7059579,7089192,
	7188715,7245095,7325355,7367748,7426778,7475903,7599231,7643174,7722266,
	7747291,7832837,7887591,7942192,8043937,8108261,8169299,8233361,8305861,
	8367181,
};

static const u32 _key[8];

static bool_t beltZerosumTest()
{
	u32 block[4];
	u32 sum[4];
	size_t i;
	// sum <- 0
	sum[0] = sum[1] = sum[2] = sum[3] = 0;
	// цикл по X_i
	for (i = 0; i < 128; ++i)
	{
		// sum <- X_i ^ Belt_0(X_i)
		block[0] = _zerosum[i];
		block[1] = block[2] = block[3] = 0;
		beltBlockEncr2(block, _key);
		sum[0] ^= _zerosum[i] ^ block[0];
		sum[1] ^= block[1];
		sum[2] ^= block[2];
		sum[3] ^= block[3];
	}
	// sum == 0?
	return sum[0] == 0 && sum[1] == 0 && sum[2] == 0 && sum[3] == 0;
}

bool_t beltTest()
{
	octet buf[48];
	octet buf1[48];
	octet mac[8];
	octet mac1[8];
	octet hash[32];
	octet hash1[32];
	u32 key[8];
	u32 block[4];
	octet level[12];
	octet state[1024];
	// создать стек
	ASSERT(sizeof(state) >= beltWBL_keep());
	ASSERT(sizeof(state) >= beltECB_keep());
	ASSERT(sizeof(state) >= beltCBC_keep());
	ASSERT(sizeof(state) >= beltCFB_keep());
	ASSERT(sizeof(state) >= beltCTR_keep());
	ASSERT(sizeof(state) >= beltMAC_keep());
	ASSERT(sizeof(state) >= beltDWP_keep());
	ASSERT(sizeof(state) >= beltKWP_keep());
	ASSERT(sizeof(state) >= beltHash_keep());
	ASSERT(sizeof(state) >= beltKRP_keep());
	ASSERT(sizeof(state) >= beltHMAC_keep());
	// тест A.1-1
	memCopy(buf, beltH(), 16);
	beltKeyExpand2(key, beltH() + 128, 32);
	beltBlockEncr(buf, key);
	if (!hexEq(buf,
		"69CCA1C93557C9E3D66BC3E0FA88FA6E"))
		return FALSE;
	beltBlockDecr(buf, key);
	if (!memEq(buf, beltH(), 16))
		return FALSE;
	// тест A.1-2
	u32From(block, beltH(), 16);
	beltBlockEncr2(block, key);
	u32To(buf, 16, block);
	if (!hexEq(buf,
		"69CCA1C93557C9E3D66BC3E0FA88FA6E"))
		return FALSE;
	beltBlockDecr2(block, key);
	u32To(buf, 16, block);
	if (!memEq(buf, beltH(), 16))
		return FALSE;
	// тест A.1-3
	beltBlockEncr3(block + 0, block + 1, block + 2, block + 3, key);
	u32To(buf, 16, block);
	if (!hexEq(buf,
		"69CCA1C93557C9E3D66BC3E0FA88FA6E"))
		return FALSE;
	beltBlockDecr3(block + 0, block + 1, block + 2, block + 3, key);
	u32To(buf, 16, block);
	if (!memEq(buf, beltH(), 16))
		return FALSE;
	// тест A.4
	memCopy(buf, beltH() + 64, 16);
	beltKeyExpand2(key, beltH() + 128 + 32, 32);
	beltBlockDecr(buf, key);
	if (!hexEq(buf,
		"0DC5300600CAB840B38448E5E993F421"))
		return FALSE;
	// тест A.6
	memCopy(buf, beltH(), 48);
	beltECBStart(state, beltH() + 128, 32);
	beltECBStepE(buf, 32, state);
	beltECBStepE(buf + 32, 48 - 32, state);
	if (!hexEq(buf,
		"69CCA1C93557C9E3D66BC3E0FA88FA6E"
		"5F23102EF109710775017F73806DA9DC"
		"46FB2ED2CE771F26DCB5E5D1569F9AB0"))
		return FALSE;
	beltECBEncr(buf1, beltH(), 48, beltH() + 128, 32);
	if (!memEq(buf, buf1, 48))
		return FALSE;
	// тест A.7
	memCopy(buf, beltH(), 47);
	beltECBStart(state, beltH() + 128, 32);
	beltECBStepE(buf, 16, state);
	beltECBStepE(buf + 16, 47 - 16, state);
	if (!hexEq(buf,
		"69CCA1C93557C9E3D66BC3E0FA88FA"
		"6E36F00CFED6D1CA1498C12798F4BE"
		"B2075F23102EF109710775017F7380"
		"6DA9"))
		return FALSE;
	beltECBEncr(buf1, beltH(), 47, beltH() + 128, 32);
	if (!memEq(buf, buf1, 47))
		return FALSE;
	// тест A.8
	memCopy(buf, beltH() + 64, 48);
	beltECBStart(state, beltH() + 128 + 32, 32);
	beltECBStepD(buf, 16, state);
	beltECBStepD(buf + 16, 48 - 16, state);
	if (!hexEq(buf,
		"0DC5300600CAB840B38448E5E993F421"
		"E55A239F2AB5C5D5FDB6E81B40938E2A"
		"54120CA3E6E19C7AD750FC3531DAEAB7"))
		return FALSE;
	beltECBDecr(buf1, beltH() + 64, 48, beltH() + 128 + 32, 32);
	if (!memEq(buf, buf1, 48))
		return FALSE;
	// тест A.9
	memCopy(buf, beltH() + 64, 36);
	beltECBStart(state, beltH() + 128 + 32, 32);
	beltECBStepD(buf, 36, state);
	if (!hexEq(buf,
		"0DC5300600CAB840B38448E5E993F421"
		"5780A6E2B69EAFBB258726D7B6718523"
		"E55A239F"))
		return FALSE;
	beltECBDecr(buf1, beltH() + 64, 36, beltH() + 128 + 32, 32);
	if (!memEq(buf, buf1, 36))
		return FALSE;
	// тест A.10
	memCopy(buf, beltH(), 48);
	beltCBCStart(state, beltH() + 128, 32, beltH() + 192);
	beltCBCStepE(buf, 32, state);
	beltCBCStepE(buf + 32, 48 - 32, state);
	if (!hexEq(buf,
		"10116EFAE6AD58EE14852E11DA1B8A74"
		"5CF2480E8D03F1C19492E53ED3A70F60"
		"657C1EE8C0E0AE5B58388BF8A68E3309"))
		return FALSE;
	beltCBCEncr(buf1, beltH(), 48, beltH() + 128, 32, beltH() + 192);
	if (!memEq(buf, buf1, 48))
		return FALSE;
	// тест A.11
	memCopy(buf, beltH(), 36);
	beltCBCStart(state, beltH() + 128, 32, beltH() + 192);
	beltCBCStepE(buf, 16, state);
	beltCBCStepE(buf + 16, 36 - 16, state);
	if (!hexEq(buf,
		"10116EFAE6AD58EE14852E11DA1B8A74"
		"6A9BBADCAF73F968F875DEDC0A44F6B1"
		"5CF2480E"))
		return FALSE;
	beltCBCEncr(buf1, beltH(), 36, beltH() + 128, 32, beltH() + 192);
	if (!memEq(buf, buf1, 36))
		return FALSE;
	// тест A.12
	memCopy(buf, beltH() + 64, 48);
	beltCBCStart(state, beltH() + 128 + 32, 32, beltH() + 192 + 16);
	beltCBCStepD(buf, 16, state);
	beltCBCStepD(buf + 16, 48 - 16, state);
	if (!hexEq(buf,
		"730894D6158E17CC1600185A8F411CAB"
		"0471FF85C83792398D8924EBD57D03DB"
		"95B97A9B7907E4B020960455E46176F8"))
		return FALSE;
	beltCBCDecr(buf1, beltH() + 64, 48, beltH() + 128 + 32, 32,
		beltH() + 192 + 16);
	if (!memEq(buf, buf1, 48))
		return FALSE;
	// тест A.13
	memCopy(buf, beltH() + 64, 36);
	beltCBCStart(state, beltH() + 128 + 32, 32, beltH() + 192 + 16);
	beltCBCStepD(buf, 16, state);
	beltCBCStepD(buf + 16, 36 - 16, state);
	if (!hexEq(buf,
		"730894D6158E17CC1600185A8F411CAB"
		"B6AB7AF8541CF85755B8EA27239F08D2"
		"166646E4"))
		return FALSE;
	beltCBCDecr(buf1, beltH() + 64, 36, beltH() + 128 + 32, 32,
		beltH() + 192 + 16);
	if (!memEq(buf, buf1, 36))
		return FALSE;
	// тест A.14
	memCopy(buf, beltH(), 48);
	beltCFBStart(state, beltH() + 128, 32, beltH() + 192);
	beltCFBStepE(buf, 16, state);
	beltCFBStepE(buf + 16, 3, state);
	beltCFBStepE(buf + 16 + 3, 48 - 16 - 3, state);
	if (!hexEq(buf,
		"C31E490A90EFA374626CC99E4B7B8540"
		"A6E48685464A5A06849C9CA769A1B0AE"
		"55C2CC5939303EC832DD2FE16C8E5A1B"))
		return FALSE;
	beltCFBEncr(buf1, beltH(), 48, beltH() + 128, 32, beltH() + 192);
	if (!memEq(buf, buf1, 48))
		return FALSE;
	// тест A.15
	memCopy(buf, beltH() + 64, 48);
	beltCFBStart(state, beltH() + 128 + 32, 32, beltH() + 192 + 16);
	beltCFBStepD(buf, 15, state);
	beltCFBStepD(buf + 15, 7, state);
	beltCFBStepD(buf + 15 + 7, 48 - 15 - 7, state);
	if (!hexEq(buf,
		"FA9D107A86F375EE65CD1DB881224BD0"
		"16AFF814938ED39B3361ABB0BF0851B6"
		"52244EB06842DD4C94AA4500774E40BB"))
		return FALSE;
	beltCFBDecr(buf1, beltH() + 64, 48, beltH() + 128 + 32, 32,
		beltH() + 192 + 16);
	if (!memEq(buf, buf1, 48))
		return FALSE;
	// тест A.16
	memCopy(buf, beltH(), 48);
	beltCTRStart(state, beltH() + 128, 32, beltH() + 192);
	beltCTRStepE(buf, 15, state);
	beltCTRStepE(buf + 15, 7, state);
	beltCTRStepE(buf + 15 + 7, 48 - 15 - 7, state);
	if (!hexEq(buf,
		"52C9AF96FF50F64435FC43DEF56BD797"
		"D5B5B1FF79FB41257AB9CDF6E63E81F8"
		"F00341473EAE409833622DE05213773A"))
		return FALSE;
	beltCTR(buf1, beltH(), 48, beltH() + 128, 32, beltH() + 192);
	if (!memEq(buf, buf1, 48))
		return FALSE;
	// тест A.17
	beltMACStart(state, beltH() + 128, 32);
	beltMACStepA(beltH(), 13, state);
	hexTo(buf, "7260DA60138F96C9");
	if (!beltMACStepV(buf, state))
		return FALSE;
	beltMAC(buf1, beltH(), 13, beltH() + 128, 32);
	if (!memEq(buf, buf1, 8))
		return FALSE;
	// тест A.18 [+ инкрементальность]
	beltMACStart(state, beltH() + 128, 32);
	beltMACStepA(beltH(), 27, state);
	beltMACStepG(buf, state);
	beltMACStepA(beltH() + 27, 48 - 27, state);
	beltMACStepG2(buf, 4, state);
	hexTo(buf, "2DAB59771B4B16D0");
	if (!beltMACStepV(buf, state) || !beltMACStepV2(buf, 3, state))
		return FALSE;
	beltMAC(buf1, beltH(), 48, beltH() + 128, 32);
	if (!memEq(buf, buf1, 8))
		return FALSE;
	// тест A.20
	beltDWPStart(state, beltH() + 128, 32, beltH() + 192);
	memCopy(buf, beltH(), 16);
	beltDWPStepE(buf, 16, state);
	beltDWPStepI(beltH() + 16, 32, state);
	beltDWPStepA(buf, 16, state);
	beltDWPStepG(mac, state);
	if (!hexEq(buf, 
		"52C9AF96FF50F64435FC43DEF56BD797"))
		return FALSE;
	if (!hexEq(mac, 
		"3B2E0AEB2B91854B"))
		return FALSE;
	beltDWPWrap(buf1, mac1, beltH(), 16, beltH() + 16, 32,
		beltH() + 128, 32, beltH() + 192);
	if (!memEq(buf, buf1, 16) || !memEq(mac, mac1, 8))
		return FALSE;
	// тест A.21
	beltDWPStart(state, beltH() + 128 + 32, 32, beltH() + 192 + 16);
	memCopy(buf, beltH() + 64, 16);
	beltDWPStepI(beltH() + 64 + 16, 32, state);
	beltDWPStepA(buf, 16, state);
	beltDWPStepD(buf, 16, state);
	beltDWPStepG(mac, state);
	if (!hexEq(buf, 
		"DF181ED008A20F43DCBBB93650DAD34B"))
		return FALSE;
	if (!hexEq(mac, 
		"6A2C2C94C4150DC0"))
		return FALSE;
	if (beltDWPUnwrap(buf1, beltH() + 64, 16, beltH() + 64 + 16, 32,
		mac, beltH() + 128 + 32, 32, beltH() + 192 + 16) != ERR_OK ||
		!memEq(buf, buf1, 16))
		return FALSE;
	// тест A.22
	beltKWPStart(state, beltH() + 128, 32);
	memCopy(buf, beltH(), 32);
	memCopy(buf + 32, beltH() + 32, 16);
	beltKWPStepE(buf, 48, state);
	if (!hexEq(buf,
		"49A38EE108D6C742E52B774F00A6EF98"
		"B106CBD13EA4FB0680323051BC04DF76"
		"E487B055C69BCF541176169F1DC9F6C8"))
		return FALSE;
	beltKWPWrap(buf1, beltH(), 32, beltH() + 32, beltH() + 128, 32);
	if (!memEq(buf, buf1, 48))
		return FALSE;
	// тест A.23
	beltKWPStart(state, beltH() + 128 + 32, 32);
	memCopy(buf, beltH() + 64, 48);
	beltKWPStepD(buf, 48, state);
	if (!hexEq(buf,
		"92632EE0C21AD9E09A39343E5C07DAA4"
		"889B03F2E6847EB152EC99F7A4D9F154"))
		return FALSE;
	if (!hexEq(buf + 32, 
		"B5EF68D8E4A39E567153DE13D72254EE"))
		return FALSE;
	if (beltKWPUnwrap(buf1, beltH() + 64, 48, (octet*)buf + 32,
		beltH() + 128 + 32, 32) != ERR_OK ||
		!memEq(buf, buf1, 32))
		return FALSE;
	// тест A.24
	beltHashStart(state);
	beltHashStepH(beltH(), 13, state);
	beltHashStepG(hash, state);
	if (!hexEq(hash,
		"ABEF9725D4C5A83597A367D14494CC25"
		"42F20F659DDFECC961A3EC550CBA8C75"))
		return FALSE;
	beltHash(hash1, beltH(), 13);
	if (!memEq(hash, hash1, 32))
		return FALSE;
	// тест A.25
	beltHashStart(state);
	beltHashStepH(beltH(), 32, state);
	hexTo(hash, 
		"749E4C3653AECE5E48DB4761227742EB"
		"6DBE13F4A80F7BEFF1A9CF8D10EE7786");
	if (!beltHashStepV(hash, state) || !beltHashStepV2(hash, 13, state))
		return FALSE;
	beltHash(hash1, beltH(), 32);
	if (!memEq(hash, hash1, 32))
		return FALSE;
	// тест A.26 [+ инкрементальность]
	beltHashStart(state);
	beltHashStepH(beltH(), 11, state);
	beltHashStepG2(hash, 32, state);
	beltHashStepH(beltH() + 11, 48 - 11, state);
	hexTo(hash, 
		"9D02EE446FB6A29FE5C982D4B13AF9D3"
		"E90861BC4CEF27CF306BFB0B174A154A");
	if (!beltHashStepV2(hash, 32, state))
		return FALSE;
	beltHash(hash1, beltH(), 48);
	if (!memEq(hash, hash1, 32))
		return FALSE;
	// тест A.29
	memSetZero(level, 12);
	level[0] = 1;
	beltKRPStart(state, beltH() + 128, 32, level);
	beltKRPStepG(buf, 16, beltH() + 32, state);
	if (!hexEq(buf, 
		"6BBBC2336670D31AB83DAA90D52C0541"))
		return FALSE;
	beltKRP(buf1, 16, beltH() + 128, 32, level, beltH() + 32);
	if (!memEq(buf, buf1, 16))
		return FALSE;
	// тест A.30
	beltKRPStepG(buf, 24, beltH() + 32, state);
	if (!hexEq(buf,
		"9A2532A18CBAF145398D5A95FEEA6C82"
		"5B9C197156A00275"))
		return FALSE;
	beltKRP(buf1, 24, beltH() + 128, 32, level, beltH() + 32);
	if (!memEq(buf, buf1, 24))
		return FALSE;
	// тест A.31
	beltKRPStepG(buf, 32, beltH() + 32, state);
	if (!hexEq(buf,
		"76E166E6AB21256B6739397B672B8796"
		"14B81CF05955FC3AB09343A745C48F77"))
		return FALSE;
	beltKRP(buf1, 32, beltH() + 128, 32, level, beltH() + 32);
	if (!memEq(buf, buf1, 32))
		return FALSE;
	// тест Б.1-1
	beltHMACStart(state, beltH() + 128, 29);
	beltHMACStepA(beltH() + 128 + 64, 32, state);
	beltHMACStepG(hash, state);
	if (!hexEq(hash,
		"D4828E6312B08BB83C9FA6535A463554"
		"9E411FD11C0D8289359A1130E930676B"))
		return FALSE;
	beltHMAC(hash1, beltH() + 128 + 64, 32, beltH() + 128, 29);
	if (!memEq(hash, hash1, 32))
		return FALSE;
	// тест Б.1-2
	beltHMACStart(state, beltH() + 128, 32);
	beltHMACStepA(beltH() + 128 + 64, 32, state);
	hexTo(hash, 
		"41FFE8645AEC0612E952D2CDF8DD508F"
		"3E4A1D9B53F6A1DB293B19FE76B1879F");
	if (!beltHMACStepV(hash, state))
		return FALSE;
	beltHMAC(hash1, beltH() + 128 + 64, 32, beltH() + 128, 32);
	if (!memEq(hash, hash1, 32))
		return FALSE;
	// тест Б.1-3 [+ икрементальность]
	beltHMACStart(state, beltH() + 128, 42);
	beltHMACStepA(beltH() + 128 + 64, 17, state);
	beltHMACStepG(hash, state);
	beltHMACStepG2(hash, 17, state);
	beltHMACStepA(beltH() + 128 + 64 + 17, 32 - 17, state);
	hexTo(hash, 
		"7D01B84D2315C332277B3653D7EC6470"
		"7EBA7CDFF7FF70077B1DECBD68F2A144");
	if (!beltHMACStepV(hash, state) || !beltHMACStepV2(hash, 23, state))
		return FALSE;
	beltHMAC(hash1, beltH() + 128 + 64, 32, beltH() + 128, 42);
	if (!memEq(hash, hash1, 32))
		return FALSE;
	// zerosum
	if (!beltZerosumTest())
		return FALSE;
	// wbl (experimental)
	{
		size_t i;
		beltWBLStart(state, beltH() + 128, 32);
		for (i = 32; i <= 48; ++i)
		{
			memCopy(buf, beltH(), i);
			beltWBLStepE(buf, i, state);
			memCopy(buf1, beltH(), i);
			beltKWPStepE(buf1, i, state);
			if (!memEq(buf, buf1, i))
				return FALSE;
			beltWBLStepD(buf, i, state);
			if (!memEq(buf, beltH(), i))
				return FALSE;
			memCopy(buf, buf1, i);
			beltKWPStepD(buf, i, state);
			beltKWPStepD2(buf1, buf1 + i - 16, i, state);
			if (!memEq(buf, buf1, i))
				return FALSE;
		}
	}
	// bde (experimental)
	{
		// тест 1
		memCopy(buf, beltH(), 48);
		beltBDEStart(state, beltH() + 128, 32, beltH() + 192);
		beltBDEStepE(buf, 32, state);
		beltBDEStepE(buf + 32, 48 - 32, state);
		if (!hexEq(buf,
			"E9CAB32D879CC50C10378EB07C10F263"
			"07257E2DBE2B854CBC9F38282D59D6A7"
			"7F952001C5D1244F53210A27C216D4BB"))
			return FALSE;
		beltBDEEncr(buf1, beltH(), 48, beltH() + 128, 32, beltH() + 192);
		if (!memEq(buf, buf1, 48))
			return FALSE;
		beltBDEDecr(buf1, buf1, 48, beltH() + 128, 32, beltH() + 192);
		if (!memEq(buf1, beltH(), 48))
			return FALSE;
		// тест 2
		memCopy(buf, beltH() + 64, 48);
		beltBDEStart(state, beltH() + 128 + 32, 32, beltH() + 192 + 16);
		beltBDEStepD(buf, 16, state);
		beltBDEStepD(buf + 16, 48 - 16, state);
		if (!hexEq(buf,
			"7041BC226352C706D00EA8EF23CFE46A"
			"FAE118577D037FACDC36E4ECC1F65746"
			"09F236943FB809E1BEE4A1C686C13ACC"))
			return FALSE;
		beltBDEDecr(buf1, beltH() + 64, 48, beltH() + 128 + 32, 32,
			beltH() + 192 + 16);
		if (!memEq(buf, buf1, 48))
			return FALSE;
		beltBDEEncr(buf, buf1, 48, beltH() + 128 + 32, 32,
			beltH() + 192 + 16);
		if (!memEq(buf, beltH() + 64, 48))
			return FALSE;
	}
	// fmt (experimental)
	{
		u16 str[21] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,};
		u16 str1[21];
		// тест 1: belt-block
		beltFMTEncrypt(str1, 10, str, 10, beltH() + 128, 32, beltH() + 192);
		beltFMTDecrypt(str1, 10, str1, 10, beltH() + 128, 32, beltH() + 192);
		if (!memEq(str, str1, 10 * 2))
			return FALSE;
		// тест 2: base58, на стыке belt-block и belt-32block
		beltFMTEncrypt(str1, 58, str, 21, beltH() + 128, 32, beltH() + 192);
		beltFMTDecrypt(str1, 58, str1, 21, beltH() + 128, 32, beltH() + 192);
		if (!memEq(str, str1, 21 * 2))
			return FALSE;
		// тест 3: на стыке belt-32block и belt-wblock
		beltFMTEncrypt(str1, 65536, str, 17, beltH() + 128, 32, beltH() + 192);
		beltFMTDecrypt(str1, 65536, str1, 17, beltH() + 128, 32, beltH() + 192);
		if (!memEq(str, str1, 17 * 2))
			return FALSE;
		// non-official
		beltFMTEncrypt(str1, 9, str, 9, beltH() + 128, 32, beltH() + 192);
		beltFMTDecrypt(str1, 9, str1, 9, beltH() + 128, 32, beltH() + 192);
		if (!memEq(str, str1, 9 * 2))
			return FALSE;
		beltFMTEncrypt(str1, 11, str, 11, beltH() + 128, 32, 0);
		beltFMTDecrypt(str1, 11, str1, 11, beltH() + 128, 32, 0);
		if (!memEq(str, str1, 11 * 2))
			return FALSE;
		beltFMTEncrypt(str1, 256, str, 16, beltH() + 128, 32, beltH() + 192);
		beltFMTDecrypt(str1, 256, str1, 16, beltH() + 128, 32, beltH() + 192);
		if (!memEq(str, str1, 16 * 2))
			return FALSE;
		beltFMTEncrypt(str1, 257, str, 17, beltH() + 128, 32, beltH() + 192);
		beltFMTDecrypt(str1, 257, str1, 17, beltH() + 128, 32, beltH() + 192);
		if (!memEq(str, str1, 17 * 2))
			return FALSE;
		beltFMTEncrypt(str1, 49667, str, 9, beltH() + 128, 32, beltH() + 192);
		beltFMTDecrypt(str1, 49667, str1, 9, beltH() + 128, 32, beltH() + 192);
		if (!memEq(str, str1, 9 * 2))
			return FALSE;
	}
	// все нормально
	return TRUE;
}
