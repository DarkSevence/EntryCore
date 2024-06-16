//Code review ~ Sevence upgrade cryptopp version 8.9.0

#include "stdafx.h"
#include "cipher.h"

#ifdef _IMPROVED_PACKET_ENCRYPTION_

#include <cryptlib.h>
#include <modes.h>
#include <nbtheory.h>
#include <osrng.h>
#include <dh.h>
#include <dh2.h>
#include <cast.h>
#include <rc6.h>
#include <mars.h>
#include <serpent.h>
#include <twofish.h>
#include <blowfish.h>
#include <camellia.h>
#include <des.h>
#include <idea.h>
#include <rc5.h>
#include <seed.h>
#include <shacal2.h>
#include <skipjack.h>
#include <tea.h>

using namespace CryptoPP;

struct BlockCipherAlgorithm 
{
	enum {kDefault, kRC6, kMARS, kTwofish, kSerpent, kCAST256, kIDEA, k3DES, kCamellia, kSEED, kRC5, kBlowfish, kTEA, kSHACAL2, kMaxAlgorithms};

	BlockCipherAlgorithm() {}
	virtual ~BlockCipherAlgorithm() {}

	static std::unique_ptr<BlockCipherAlgorithm> Pick(int32_t hint);										 

	virtual int32_t GetBlockSize() const = 0;
	virtual int32_t GetDefaultKeyLength() const = 0;
	virtual int32_t GetIVLength() const = 0;

	virtual SymmetricCipher* CreateEncoder(const byte* key, size_t keylen, const byte* iv) const = 0;
	virtual SymmetricCipher* CreateDecoder(const byte* key, size_t keylen, const byte* iv) const = 0;
};

template<class T>
struct BlockCipherDetail : public BlockCipherAlgorithm 
{
	BlockCipherDetail() {}
	virtual ~BlockCipherDetail() {}

	virtual int32_t GetBlockSize() const 
	{ 
		return T::BLOCKSIZE; 
	}
	
	virtual int32_t GetDefaultKeyLength() const 
	{ 
		return T::DEFAULT_KEYLENGTH; 
	}
	
	virtual int32_t GetIVLength() const 
	{ 
		return T::IV_LENGTH; 
	}

	virtual SymmetricCipher* CreateEncoder(const byte* key, size_t keylen, const byte* iv) const 
	{
		return new typename CTR_Mode<T>::Encryption(key, keylen, iv);
	}
  
	virtual SymmetricCipher* CreateDecoder(const byte* key, size_t keylen, const byte* iv) const 
	{
		return new typename CTR_Mode<T>::Decryption(key, keylen, iv);
	}
};

class KeyAgreement 
{
	public:
		KeyAgreement() {}
		virtual ~KeyAgreement() {}
		virtual size_t Prepare(void* buffer, size_t* length) = 0;
		virtual bool Agree(size_t agreed_length, const void* buffer, size_t length) = 0;

		const SecByteBlock& shared() const 
		{ 
			return shared_;
		}

	protected:
		SecByteBlock shared_;
};

class DH2KeyAgreement : public KeyAgreement 
{
	public:
		DH2KeyAgreement();
		virtual ~DH2KeyAgreement();
		virtual size_t Prepare(void* buffer, size_t* length);
		virtual bool Agree(size_t agreed_length, const void* buffer, size_t length);
		
	private:
		DH dh_;
		DH2 dh2_;
		SecByteBlock spriv_key_;
		SecByteBlock epriv_key_;
};

Cipher::Cipher() : activated_(false), encoder_(nullptr), decoder_(nullptr), key_agreement_(nullptr) {}

Cipher::~Cipher() 
{
	if (activated_) 
	{
		CleanUp();
	}
}

void Cipher::CleanUp() 
{
	encoder_.reset();
	decoder_.reset();
	key_agreement_.reset();
	activated_ = false;
}

size_t Cipher::Prepare(void* buffer, size_t* length) 
{
	assert(!key_agreement_);
	key_agreement_ = std::make_unique<DH2KeyAgreement>();
	assert(key_agreement_);
	
	size_t agreed_length = key_agreement_->Prepare(buffer, length);
	
	if (agreed_length == 0) 
	{
		key_agreement_.reset();
	}
	
	return agreed_length;
}

bool Cipher::Activate(bool polarity, size_t agreed_length, const void* buffer, size_t length) 
{
	assert(!activated_);
	assert(key_agreement_);

	if (!activated_ && key_agreement_->Agree(agreed_length, buffer, length)) 
	{
		activated_ = SetUp(polarity);
	}

	key_agreement_.reset();
	return activated_;
}

bool Cipher::SetUp(bool polarity)
{
	assert(key_agreement_ != nullptr);
	const SecByteBlock& shared = key_agreement_->shared();

	if (shared.size() < 2)
	{
		return false;
	}
	
	int32_t hint_0 = shared.BytePtr()[shared.BytePtr()[0] % shared.size()];
	int32_t hint_1 = shared.BytePtr()[shared.BytePtr()[1] % shared.size()];
	
	auto algorithm_0 = std::unique_ptr<BlockCipherAlgorithm>(BlockCipherAlgorithm::Pick(hint_0));
	auto algorithm_1 = std::unique_ptr<BlockCipherAlgorithm>(BlockCipherAlgorithm::Pick(hint_1));
	
	assert(algorithm_0 != nullptr && algorithm_1 != nullptr);

	const size_t key_length_0 = algorithm_0->GetDefaultKeyLength();
	const size_t iv_length_0 = algorithm_0->GetBlockSize();
	const size_t key_length_1 = algorithm_1->GetDefaultKeyLength();
	const size_t iv_length_1 = algorithm_1->GetBlockSize();

	if (shared.size() < std::max({key_length_0, iv_length_0, key_length_1, iv_length_1}))
	{
		return false;
	}

	SecByteBlock key_0(key_length_0), iv_0(iv_length_0);
	SecByteBlock key_1(key_length_1), iv_1(iv_length_1);

	key_0.Assign(shared, key_length_0);
	size_t offset = std::min(key_length_0, shared.size() - key_length_1);
	key_1.Assign(shared.BytePtr() + offset, key_length_1);

	offset = shared.size() - iv_length_0;
	iv_0.Assign(shared.BytePtr() + offset, iv_length_0);
	offset = (offset < iv_length_1 ? 0 : offset - iv_length_1);
	iv_1.Assign(shared.BytePtr() + offset, iv_length_1);

	if (polarity)
	{
		encoder_.reset(algorithm_1->CreateEncoder(key_1, key_1.size(), iv_1));
		decoder_.reset(algorithm_0->CreateDecoder(key_0, key_0.size(), iv_0));
	}
	else
	{
		encoder_.reset(algorithm_0->CreateEncoder(key_0, key_0.size(), iv_0));
		decoder_.reset(algorithm_1->CreateDecoder(key_1, key_1.size(), iv_1));
	}

	assert(encoder_ != nullptr && decoder_ != nullptr);
	return true;
}

std::unique_ptr<BlockCipherAlgorithm> BlockCipherAlgorithm::Pick(int32_t hint) 
{
	int32_t selector = hint % kMaxAlgorithms;
	switch (selector) 
	{
		case kRC6:
		{
			return std::make_unique<BlockCipherDetail<RC6>>();
		}
		
		case kMARS:
		{
			return std::make_unique<BlockCipherDetail<MARS>>();
		}
		
		case kTwofish:
		{
			return std::make_unique<BlockCipherDetail<Twofish>>();
		}
		
		case kSerpent:
		{
			return std::make_unique<BlockCipherDetail<Serpent>>();
		}
		
		case kCAST256:
		{
			return std::make_unique<BlockCipherDetail<CAST256>>();
		}
		
		case kIDEA:
		{
			return std::make_unique<BlockCipherDetail<IDEA>>();
		}
		
		case k3DES:
		{
			return std::make_unique<BlockCipherDetail<DES_EDE2>>();
		}
		
		case kCamellia:
		{
			return std::make_unique<BlockCipherDetail<Camellia>>();
		}
		
		case kSEED:
		{
			return std::make_unique<BlockCipherDetail<SEED>>();
		}
		
		case kRC5:
		{
			return std::make_unique<BlockCipherDetail<RC5>>();
		}
		
		case kBlowfish:
		{
			return std::make_unique<BlockCipherDetail<Blowfish>>();
		}
		
		case kTEA:
		{
			return std::make_unique<BlockCipherDetail<TEA>>();
		}
		
		case kSHACAL2:
		{
			return std::make_unique<BlockCipherDetail<SHACAL2>>();
		}
		
		case kDefault:
		
		default:
		{
			return std::make_unique<BlockCipherDetail<Twofish>>();
		}
	}
}

DH2KeyAgreement::DH2KeyAgreement() : dh_(), dh2_(dh_) {}
DH2KeyAgreement::~DH2KeyAgreement() {}

size_t DH2KeyAgreement::Prepare(void* buffer, size_t* length)
{
	Integer p("0xB10B8F96A080E01DDE92DE5EAE5D54EC52C99FBCFB06A3C6"
			  "9A6A9DCA52D23B616073E28675A23D189838EF1E2EE652C0"
			  "13ECB4AEA906112324975C3CD49B83BFACCBDD7D90C4BD70"
			  "98488E9C219A73724EFFD6FAE5644738FAA31A4FF55BCCC0"
			  "A151AF5F0DC8B4BD45BF37DF365C1A65E68CFDA76D4DA708"
			  "DF1FB2BC2E4A4371");

	Integer g("0xA4D1CBD5C3FD34126765A442EFB99905F8104DD258AC507F"
			  "D6406CFF14266D31266FEA1E5C41564B777E690F5504F213"
			  "160217B4B01B886A5E91547F9E2749F4D7FBD7D3B9A92EE1"
			  "909D0D2263F80A76A6A24C087A091F531DBF0A0169B6A28A"
			  "D662A4D18E73AFA32D779D5918D08BC8858F4DCEF97C2A24"
			  "855E6EEB22B3B2E5");

	Integer q("0xF518AA8781A8DF278ABA4E7D64B7CB9D49462353");

	AutoSeededRandomPool rnd;

	dh_.AccessGroupParameters().Initialize(p, q, g);

	if(!dh_.GetGroupParameters().ValidateGroup(rnd, 3))
	{
		return 0;
	}

	p = dh_.GetGroupParameters().GetModulus();
	q = dh_.GetGroupParameters().GetSubgroupOrder();
	g = dh_.GetGroupParameters().GetGenerator();

	if (ModularExponentiation(g, q, p) != Integer::One())
	{
		return 0;
	}
	
	spriv_key_.New(dh2_.StaticPrivateKeyLength());
	epriv_key_.New(dh2_.EphemeralPrivateKeyLength());
	SecByteBlock spub_key(dh2_.StaticPublicKeyLength());
	SecByteBlock epub_key(dh2_.EphemeralPublicKeyLength());

	dh2_.GenerateStaticKeyPair(rnd, spriv_key_, spub_key);
	dh2_.GenerateEphemeralKeyPair(rnd, epriv_key_, epub_key);

	const size_t spub_key_length = spub_key.size();
	const size_t epub_key_length = epub_key.size();
	const size_t data_length = spub_key_length + epub_key_length;
	
	if (*length < data_length || buffer == nullptr)
	{
		return 0;
	}
	
	*length = data_length;
	BYTE* buf = (BYTE*)buffer;

	std::memcpy(buffer, spub_key.BytePtr(), spub_key_length);
	std::memcpy(static_cast<byte*>(buffer) + spub_key_length, epub_key.BytePtr(), epub_key_length);

	return dh2_.AgreedValueLength();
}

bool DH2KeyAgreement::Agree(size_t agreed_length, const void* buffer, size_t length)
{
	if (agreed_length != dh2_.AgreedValueLength())
	{
		return false;
	}
	
	const size_t spub_key_length = dh2_.StaticPublicKeyLength();
	const size_t epub_key_length = dh2_.EphemeralPublicKeyLength();
	
	if (length != (spub_key_length + epub_key_length))
	{
		return false;
	}
	
	auto shared_buffer = std::make_unique<byte[]>(dh2_.AgreedValueLength());
	const byte* buf = static_cast<const byte*>(buffer);
	
	if (!dh2_.Agree(shared_buffer.get(), spriv_key_, epriv_key_, buf, buf + spub_key_length))
	{
		return false;
	}

	shared_.Assign(shared_buffer.get(), agreed_length);
	return true;
}
#endif