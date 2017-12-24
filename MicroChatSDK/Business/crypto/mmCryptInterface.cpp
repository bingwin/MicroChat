#include "mmCryptInterface.h"
#include "mars/comm/windows/zlib/zlib.h"
#include "aes.h"
#include "rsa.h"
#include "ec.h"
#include "ecdh.h"
#include "mars/openssl/include/openssl/md5.h"



std::string compress_aes(string key, string in, DWORD &dwLenCompressed)
{
	string strOut;

	string strCompressed;
	dwLenCompressed = compressBound(in.size());
	strCompressed.resize(dwLenCompressed);
	int ret = compress((Bytef *)strCompressed.c_str(), &dwLenCompressed, (const Bytef *)in.c_str(), in.size());
	if (ret)	return strOut;
	
	strCompressed.resize(dwLenCompressed);

	AES_KEY aesKey;
	string iv = key;

	unsigned int uiPaddingLen;
	unsigned int uiTotalLen;
	unsigned char* pData;

	ret = AES_set_encrypt_key((const unsigned char *)key.c_str(), key.size()*8, &aesKey);
	if (ret)	return strOut;

	//padding
	uiPaddingLen = key.size() - (strCompressed.size() % key.size());
	uiTotalLen = strCompressed.size() + uiPaddingLen;
	pData = (unsigned char*)malloc(uiTotalLen);
	if (!pData)	return strOut;

	memcpy(pData, strCompressed.c_str(), strCompressed.size());

	if (uiPaddingLen > 0) memset(pData + strCompressed.size(), uiPaddingLen, uiPaddingLen);

	strOut.resize(uiTotalLen*2);
	AES_cbc_encrypt(pData, (unsigned char*)strOut.c_str(), uiTotalLen, &aesKey, (unsigned char *)iv.c_str(), AES_ENCRYPT);
	free(pData);
	strOut.resize(uiTotalLen);

	return strOut;
}

std::string nocompress_aes(string key, string in)
{
	string strOut;

	string strCompressed = in;

	AES_KEY aesKey;
	string iv = key;

	unsigned int uiPaddingLen;
	unsigned int uiTotalLen;
	unsigned char* pData;

	int ret = AES_set_encrypt_key((const unsigned char *)key.c_str(), key.size() * 8, &aesKey);
	if (ret)	return strOut;

	//padding
	uiPaddingLen = key.size() - (strCompressed.size() % key.size());
	uiTotalLen = strCompressed.size() + uiPaddingLen;
	pData = (unsigned char*)malloc(uiTotalLen);
	if (!pData)	return strOut;

	memcpy(pData, strCompressed.c_str(), strCompressed.size());

	if (uiPaddingLen > 0) memset(pData + strCompressed.size(), uiPaddingLen, uiPaddingLen);

	strOut.resize(uiTotalLen * 2);
	AES_cbc_encrypt(pData, (unsigned char*)strOut.c_str(), uiTotalLen, &aesKey, (unsigned char *)iv.c_str(), AES_ENCRYPT);
	free(pData);
	strOut.resize(uiTotalLen);

	return strOut;
}

std::string compress_rsa(string keye, string keyn, string in, DWORD &dwLenCompressed)
{
	string strOut;

	string strCompressed;
	dwLenCompressed = compressBound(in.size());
	strCompressed.resize(dwLenCompressed);
	int ret = compress((Bytef *)strCompressed.c_str(), &dwLenCompressed, (const Bytef *)in.c_str(), in.size());
	if (ret)	return strOut;

	strCompressed.resize(dwLenCompressed);


	RSA* rsa = RSA_new();
	if (rsa == NULL)	return strOut;

	if (BN_hex2bn(&rsa->n, keyn.c_str()) == 0)		goto END;
	if (BN_hex2bn(&rsa->e, keye.c_str()) == 0)		goto END;

	int rsa_len = RSA_size(rsa);

	if ((int)strCompressed.size()>= rsa_len - 12) 
	{
		int blockCnt = (strCompressed.size() / (rsa_len - 12)) + (((strCompressed.size() % (rsa_len - 12)) == 0) ? 0 : 1);
		strOut.resize(blockCnt * rsa_len);

		for (int i = 0; i < blockCnt; ++i) {
			int blockSize = rsa_len - 12;
			if (i == blockCnt - 1) blockSize = strCompressed.size() - i * blockSize;

			if (RSA_public_encrypt(blockSize, ((const unsigned char *)strCompressed.c_str() + i * (rsa_len - 12)), ((unsigned char *)strOut.c_str() + i * rsa_len), rsa, RSA_PKCS1_PADDING) < 0) 
			{
				strOut.resize(0);
			}
		}
	}
	else {
		strOut.resize(rsa_len);

		if (RSA_public_encrypt(strCompressed.size(),(const unsigned char *)strCompressed.c_str(), (unsigned char *)strOut.c_str(), rsa, RSA_PKCS1_PADDING) < 0)
		{
			strOut.resize(0);
		}
	}

END:
	RSA_free(rsa);
	
	return strOut;
}

std::string nocompress_rsa(string keye, string keyn, string in)
{
	string strOut;

	string strCompressed = in;

	RSA* rsa = RSA_new();
	if (rsa == NULL)	return strOut;

	if (BN_hex2bn(&rsa->n, keyn.c_str()) == 0)		goto END;
	if (BN_hex2bn(&rsa->e, keye.c_str()) == 0)		goto END;

	int rsa_len = RSA_size(rsa);

	if ((int)strCompressed.size() >= rsa_len - 12)
	{
		int blockCnt = (strCompressed.size() / (rsa_len - 12)) + (((strCompressed.size() % (rsa_len - 12)) == 0) ? 0 : 1);
		strOut.resize(blockCnt * rsa_len);

		for (int i = 0; i < blockCnt; ++i) {
			int blockSize = rsa_len - 12;
			if (i == blockCnt - 1) blockSize = strCompressed.size() - i * blockSize;

			if (RSA_public_encrypt(blockSize, ((const unsigned char *)strCompressed.c_str() + i * (rsa_len - 12)), ((unsigned char *)strOut.c_str() + i * rsa_len), rsa, RSA_PKCS1_PADDING) < 0)
			{
				strOut.resize(0);
			}
		}
	}
	else {
		strOut.resize(rsa_len);

		if (RSA_public_encrypt(strCompressed.size(), (const unsigned char *)strCompressed.c_str(), (unsigned char *)strOut.c_str(), rsa, RSA_PKCS1_PADDING) < 0)
		{
			strOut.resize(0);
		}
	}

END:
	RSA_free(rsa);

	return strOut;
}

std::string aes_uncompress(string key, string in,DWORD dwLenOut)
{
	string strOut;

	if (!in.size())	return strOut;

	AES_KEY aesKey;
	string iv = key;

	unsigned int uiTotalLen;

	int ret = AES_set_decrypt_key((const unsigned char *)key.c_str(), key.size() * 8, &aesKey);
	if (ret)	return strOut;
	
	string strCompressed;
	strCompressed.resize(in.size());
	AES_cbc_encrypt((const unsigned char *)in.c_str(), (unsigned char*)strCompressed.c_str(), in.size(), &aesKey, (unsigned char *)iv.c_str(), AES_DECRYPT);

	unsigned int uiPaddingLen = strCompressed[in.size() - 1];
	if (uiPaddingLen > key.size() || uiPaddingLen < 0) 
	{
		return strOut;
	}
	strCompressed.resize(in.size() - uiPaddingLen);

	DWORD dwLenUncompress = dwLenOut;
	strOut.resize(dwLenUncompress);
	ret = uncompress((unsigned char *)strOut.c_str(), &dwLenUncompress, (const unsigned char*)strCompressed.c_str(), strCompressed.size());
	if (ret || dwLenUncompress != dwLenOut)
	{
		strOut.resize(0);
		return strOut;
	}

	return strOut;
}

std::string aes_nouncompress(string key, string in)
{
	string strOut;

	if (!in.size())	return strOut;

	AES_KEY aesKey;
	string iv = key;

	unsigned int uiTotalLen;

	int ret = AES_set_decrypt_key((const unsigned char *)key.c_str(), key.size() * 8, &aesKey);
	if (ret)	return strOut;

	string strCompressed;
	strCompressed.resize(in.size());
	AES_cbc_encrypt((const unsigned char *)in.c_str(), (unsigned char*)strCompressed.c_str(), in.size(), &aesKey, (unsigned char *)iv.c_str(), AES_DECRYPT);

	unsigned int uiPaddingLen = strCompressed[in.size() - 1];
	if (uiPaddingLen > key.size() || uiPaddingLen < 0)
	{
		return strOut;
	}
	strCompressed.resize(in.size() - uiPaddingLen);

	strOut = strCompressed;

	return strOut;
}

bool GenEcdh(int nid,string &strPubKey, string &strPriKey)
{
	EC_KEY *ec_key = EC_KEY_new_by_curve_name(nid);
	if (!ec_key) return FALSE;

	int ret = EC_KEY_generate_key(ec_key);
	if (1 != ret)
	{
		EC_KEY_free(ec_key);
		ec_key = NULL;
		return FALSE;
	}
	int nLenPub = i2o_ECPublicKey(ec_key, NULL);
	strPubKey.resize(nLenPub);
	unsigned char *pub_key_buf = NULL;
	ret = i2o_ECPublicKey(ec_key, &pub_key_buf);
	if (!ret)
	{
		EC_KEY_free(ec_key);
		ec_key = NULL;
		return FALSE;
	}
	strPubKey = std::string((const char *)pub_key_buf, nLenPub);

	int nLenPri = i2d_ECPrivateKey(ec_key, NULL);
	strPriKey.resize(nLenPri);
	unsigned char *pri_key_buf = NULL;
	ret = i2d_ECPrivateKey(ec_key, &pri_key_buf);
	if (!ret)
	{
		EC_KEY_free(ec_key);
		ec_key = NULL;
		return FALSE;
	}
	strPriKey = std::string((const char *)pri_key_buf, nLenPri);

	if (ec_key)
	{
		EC_KEY_free(ec_key);
		ec_key = NULL;
	}
	if (pub_key_buf)
	{
		OPENSSL_free(pub_key_buf);
	}
	if (pri_key_buf)
	{
		OPENSSL_free(pri_key_buf);
	}
	
	return TRUE;
}

static void *KDF_MD5(const void *in, size_t inlen, void *out, size_t *outlen)
{
	if (!out || !outlen || *outlen < MD5_DIGEST_LENGTH)	return NULL;
	
	string strIn = string((const char*)in,inlen);
	string strOut = GetMd5(strIn);

	if (MD5_DIGEST_LENGTH == strOut.size())
	{
		memcpy(out, strOut.c_str(), MD5_DIGEST_LENGTH);
		*outlen = MD5_DIGEST_LENGTH;
	}
	else
	{
		out = NULL;
		*outlen = 0;
	}

	return out;	
}

std::string DoEcdh(int nid, string &strServerPubKey, string &strLocalPriKey)
{	
	string strEcdh;

	const unsigned char *public_material = (const unsigned char *)strServerPubKey.c_str();
	const unsigned char *private_material = (const unsigned char *)strLocalPriKey.c_str();

	EC_KEY *pub_ec_key = EC_KEY_new_by_curve_name(nid);
	if (!pub_ec_key)	return strEcdh;
	pub_ec_key = o2i_ECPublicKey(&pub_ec_key, &public_material, strServerPubKey.size());
	if (!pub_ec_key)	return strEcdh;

	EC_KEY *pri_ec_key = EC_KEY_new_by_curve_name(nid);
	if (!pri_ec_key)	return strEcdh;
	pri_ec_key = d2i_ECPrivateKey(&pri_ec_key, &private_material, strLocalPriKey.size());
	if (!pri_ec_key) return strEcdh;

	strEcdh.resize(MD5_DIGEST_LENGTH);

	if (MD5_DIGEST_LENGTH != ECDH_compute_key((void *)strEcdh.c_str(), MD5_DIGEST_LENGTH, EC_KEY_get0_public_key(pub_ec_key), pri_ec_key, KDF_MD5))
	{
		EC_KEY_free(pub_ec_key);
		EC_KEY_free(pri_ec_key);
	}

	if (pub_ec_key) 
	{
		EC_KEY_free(pub_ec_key);
		pub_ec_key = NULL;
	}

	if (pri_ec_key) 
	{
		EC_KEY_free(pri_ec_key);
		pri_ec_key = NULL;
	}

	return strEcdh;
}

std::string GetMd5(string in)
{
	string strMd5;
	
	if (!in.size())	return strMd5;

	strMd5.resize(MD5_DIGEST_LENGTH);

	MD5_CTX ctx;
	MD5_Init(&ctx);
	MD5_Update(&ctx, in.c_str(), in.size());
	MD5_Final((unsigned char*)strMd5.c_str(), &ctx);

	return strMd5;
}

std::string GetMd5_32(string in)
{
	string md5 = GetMd5(in);

	string strMd5;
	strMd5.resize(32);
	
	char szMd5[33] = { 0 };
	for (int i=0;i<md5.size();i++)
	{	
		sprintf(&szMd5[i*2],"%02x", (unsigned char)md5[i]); 
	}
	strMd5 = (const char *)szMd5;

	return strMd5;
}
