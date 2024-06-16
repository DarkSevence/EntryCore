#pragma once

#ifdef _IMPROVED_PACKET_ENCRYPTION_
#include <cryptlib.h>

class KeyAgreement;
class Cipher 
{
	public:
		Cipher();
		~Cipher();

		void CleanUp();

		size_t Prepare(void* buffer, size_t* length);
		bool Activate(bool polarity, size_t agreed_length,
		const void* buffer, size_t length);

		void Encrypt(void* buffer, size_t length)
		{
			assert(activated_);
			encoder_->ProcessData((CryptoPP::byte*)buffer, (const CryptoPP::byte*)buffer, length);
		}
		
		void Decrypt(void* buffer, size_t length)
		{
			assert(activated_);
			decoder_->ProcessData((CryptoPP::byte*)buffer, (const CryptoPP::byte*)buffer, length);
		}

		bool activated() const { return activated_; }
		void set_activated(bool value) { activated_ = value; }

		bool IsKeyPrepared() { return key_agreement_ != nullptr; }

	private:
		bool SetUp(bool polarity);
		bool activated_;

		std::unique_ptr<CryptoPP::SymmetricCipher> encoder_;
		std::unique_ptr<CryptoPP::SymmetricCipher> decoder_;
		std::unique_ptr<KeyAgreement> key_agreement_;
};
#endif


