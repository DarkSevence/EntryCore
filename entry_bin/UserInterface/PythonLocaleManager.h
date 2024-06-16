#pragma once

template <typename T>
class CSingleton;

class PythonLocaleManager : public CSingleton<PythonLocaleManager>
{
	public:
		PythonLocaleManager();
		virtual ~PythonLocaleManager();

		enum class LocaleStringType
		{
			GENERAL,
			QUEST,
			OXQUIZ,
			MaxTypes
		};

		using TokenVector = std::vector<std::string>;

		bool LoadGeneralLocaleStrings(const char* fileName);
		bool LoadQuestLocaleStrings(const char* fileName);
		bool LoadOXQuizLocaleStrings(const char* fileName);
		void FormatLocaleString(std::string& formattedMessage) const;
		void FormatLocaleString(char* destinationBuffer, size_t bufferSize) const;
		void SplitIntoMultipleLines(const std::string& message, TokenVector& tokens) const;

	private:
		void ReplaceSkillNamesInString(std::string& message) const;
		void ReplaceMobNamesInString(std::string& message) const;
		void ReplaceItemNamesInString(std::string& message) const;
		void ReplaceGeneralLocaleStrings(std::string& message) const;
		void ReplaceQuestLocaleStrings(std::string& message) const;
		void ReplaceOXQuizLocaleStrings(std::string& message) const;

		std::unordered_map<std::string, std::string> localeStringMaps[static_cast<int>(LocaleStringType::MaxTypes)];
};