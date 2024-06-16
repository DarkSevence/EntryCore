#include "StdAfx.h"
#include "PythonSkill.h"
#include "PythonLocaleManager.h"
#include "PythonNonPlayer.h"
#include "../GameLib/ItemManager.h"
#include "../eterPack/EterPackManager.h"

#undef snprintf
#include <boost/format.hpp>
#define snprintf _snprintf

PythonLocaleManager::PythonLocaleManager() = default;
PythonLocaleManager::~PythonLocaleManager() = default;

bool PythonLocaleManager::LoadGeneralLocaleStrings(const char* fileName)
{
	if (!localeStringMaps[static_cast<int>(LocaleStringType::GENERAL)].empty())
	{
		return true;
	}

	const VOID* dataPointer;
	CMappedFile mappedFile;

	if (!CEterPackManager::Instance().Get(mappedFile, fileName, &dataPointer))
	{
		TraceError("PythonLocaleManager::LoadGeneralLocaleStrings(fileName: %s) - File does not exist", fileName);
		return false;
	}

	CMemoryTextFileLoader textFileLoader;
	textFileLoader.Bind(mappedFile.Size(), dataPointer);

	TokenVector tokenVector;
	auto& targetMap = localeStringMaps[static_cast<int>(LocaleStringType::GENERAL)];

	uint32_t lineCount = textFileLoader.GetLineCount();
	for (uint32_t i = 0; i < lineCount; ++i)
	{
		const std::string& lineContent = textFileLoader.GetLineString(i);

		if (lineContent.empty() || lineContent[0] == '#' || !textFileLoader.SplitLineByTab(i, &tokenVector))
		{
			continue;
		}

		if (tokenVector.size() == 2)
		{
			targetMap.try_emplace(tokenVector[0], tokenVector[1]);
		}
	}

	return true;
}

bool PythonLocaleManager::LoadQuestLocaleStrings(const char* fileName)
{
	if (!localeStringMaps[static_cast<int>(LocaleStringType::QUEST)].empty())
	{
		return true;
	}

	const void* pvData;
	CMappedFile kFile;

	if (!CEterPackManager::Instance().Get(kFile, fileName, &pvData))
	{
		TraceError("CPythonLocale::LoadQuestLocaleString(c_szFileName: %s) - File does not exist", fileName);
		return false;
	}

	CMemoryTextFileLoader kTextFileLoader;
	kTextFileLoader.Bind(kFile.Size(), pvData);

	CTokenVector kTokenVector;
	auto& targetMap = localeStringMaps[static_cast<int>(LocaleStringType::QUEST)];

	uint32_t lineCount = kTextFileLoader.GetLineCount();
	for (uint32_t i = 0; i < lineCount; ++i)
	{
		const std::string& c_rstLine = kTextFileLoader.GetLineString(i);
		if (c_rstLine.empty() || c_rstLine[0] == '#' || !kTextFileLoader.SplitLineByTab(i, &kTokenVector))
		{
			continue;
		}

		if (kTokenVector.size() == 2)
		{
			targetMap.try_emplace(kTokenVector[0], kTokenVector[1]);
		}
	}

	return true;
}

bool PythonLocaleManager::LoadOXQuizLocaleStrings(const char* fileName)
{
	if (!localeStringMaps[static_cast<int>(LocaleStringType::OXQUIZ)].empty())
	{
		return true;
	}

	const void* pvData;
	CMappedFile kFile;

	if (!CEterPackManager::Instance().Get(kFile, fileName, &pvData))
	{
		TraceError("CPythonLocale::LoadOXQuizLocaleString(c_szFileName: %s) - Load Error", fileName);
		return false;
	}

	CMemoryTextFileLoader kTextFileLoader;
	kTextFileLoader.Bind(kFile.Size(), pvData);

	CTokenVector kTokenVector;
	auto& targetMap = localeStringMaps[static_cast<int>(LocaleStringType::OXQUIZ)];

	uint32_t lineCount = kTextFileLoader.GetLineCount();
	for (uint32_t i = 0; i < lineCount; ++i)
	{
		const std::string& c_rstLine = kTextFileLoader.GetLineString(i);
		if (c_rstLine.empty() || c_rstLine[0] == '#' || !kTextFileLoader.SplitLineByTab(i, &kTokenVector))
		{
			continue;
		}

		if (kTokenVector.size() == 3)
		{
			targetMap.try_emplace(kTokenVector[1], kTokenVector[2]);
		}
	}

	return true;
}

void PythonLocaleManager::FormatLocaleString(std::string& message) const
{
	ReplaceSkillNamesInString(message);
	ReplaceMobNamesInString(message);
	ReplaceItemNamesInString(message);
	ReplaceGeneralLocaleStrings(message);
	ReplaceQuestLocaleStrings(message);
	ReplaceOXQuizLocaleStrings(message);
}

void PythonLocaleManager::FormatLocaleString(char* destBuffer, size_t bufferSize) const
{
	if (destBuffer == nullptr || bufferSize == 0)
	{
		return;
	}

	std::string formatString{ destBuffer };
	FormatLocaleString(formatString);

	if (bufferSize <= formatString.size())
	{
		std::memcpy(destBuffer, formatString.data(), bufferSize - 1);
		destBuffer[bufferSize - 1] = '\0';
	}
	else
	{
		std::memcpy(destBuffer, formatString.data(), formatString.size());
		destBuffer[formatString.size()] = '\0';
	}
}

void PythonLocaleManager::SplitIntoMultipleLines(const std::string& message, TokenVector& tokens) const
{
	constexpr const char* enterToken = "[ENTER]";
	constexpr size_t enterTokenLength = 7;

	if (message.find(enterToken) == std::string::npos)
	{
		return;
	}

	size_t startPos = 0;
	while (true)
	{
		const size_t endPos = message.find(enterToken, startPos);
		if (endPos == std::string::npos)
		{
			break;
		}

		tokens.emplace_back(message.substr(startPos, endPos - startPos));
		startPos = endPos + enterTokenLength;
	}

	if (startPos < message.size())
	{
		tokens.emplace_back(message.substr(startPos));
	}
}

void PythonLocaleManager::ReplaceSkillNamesInString(std::string& message) const
{
	constexpr const char* skillTokenBegin = "[SN;";
	constexpr size_t skillTokenBeginLength = 4;

	size_t startPos = 0;

	while (true)
	{
		const size_t posBegin = message.find(skillTokenBegin, startPos);
		if (posBegin == std::string::npos)
		{
			break;
		}

		const size_t posMid = message.find(';', posBegin);
		if (posMid == std::string::npos)
		{
			break;
		}

		const size_t posEnd = message.find(']', posMid + 1);
		if (posEnd == std::string::npos)
		{
			break;
		}

		uint32_t skillVnum{};
		try
		{
			skillVnum = std::stoul(message.substr(posMid + 1, posEnd - posMid - 1));
		}
		catch (const std::exception& ex)
		{
			TraceError("CPythonLocale::ReplaceSkillName: Error: %s", ex.what());
			break;
		}

		CPythonSkill::TSkillData* skillData;
		if (!CPythonSkill::Instance().GetSkillData(skillVnum, &skillData))
		{
			TraceError("CPythonLocale::ReplaceSkillName: can't find skill vnum: %u", skillVnum);
			break;
		}

		message.replace(posBegin, posEnd - posBegin + 1, skillData->strName);
		startPos = posBegin + skillData->strName.size();
	}
}

void PythonLocaleManager::ReplaceMobNamesInString(std::string& message) const
{
	constexpr std::string_view mobTokenBegin = "[MN;";
	constexpr size_t mobTokenBeginLength = 4;

	size_t startPos = 0;

	while (true)
	{
		const size_t posBegin = message.find(mobTokenBegin, startPos);
		if (posBegin == std::string::npos)
		{
			break;
		}

		const size_t posMid = message.find(';', posBegin);
		if (posMid == std::string::npos)
		{
			break;
		}

		const size_t posEnd = message.find(']', posMid + 1);
		if (posEnd == std::string::npos)
		{
			break;
		}

		uint32_t mobVnum{};
		try
		{
			mobVnum = std::stoul(message.substr(posMid + 1, posEnd - posMid - 1));
		}
		catch (const std::exception& ex)
		{
			TraceError("CPythonLocale::ReplaceMobName: Error: %s", ex.what());
			break;
		}

		const char* mobName;
		if (!CPythonNonPlayer::Instance().GetName(mobVnum, &mobName))
		{
			TraceError("CPythonLocale::ReplaceMobName: can't find mob vnum: %u", mobVnum);
			break;
		}

		const std::string_view mobNameView(mobName);
		message.replace(posBegin, posEnd - posBegin + 1, mobNameView);
		startPos = posBegin + mobNameView.size();
	}
}

void PythonLocaleManager::ReplaceItemNamesInString(std::string& message) const
{
	constexpr std::string_view itemTokenBegin = "[IN;";
	constexpr size_t itemTokenBeginLength = 4;

	size_t startPos = 0;

	while (true)
	{
		const size_t posBegin = message.find(itemTokenBegin, startPos);
		if (posBegin == std::string::npos)
		{
			break;
		}

		const size_t posMid = message.find(';', posBegin);
		if (posMid == std::string::npos)
		{
			break;
		}

		const size_t posEnd = message.find(']', posMid + 1);
		if (posEnd == std::string::npos)
		{
			break;
		}

		uint32_t itemVnum{};
		try
		{
			itemVnum = std::stoul(message.substr(posMid + 1, posEnd - posMid - 1));
		}
		catch (const std::exception& ex)
		{
			TraceError("CPythonLocale::ReplaceItemName: Error: %s", ex.what());
			break;
		}

		CItemData* itemData;
		if (!CItemManager::Instance().GetItemDataPointer(itemVnum, &itemData))
		{
			TraceError("CPythonLocale::ReplaceItemName: can't find item vnum: %u", itemVnum);
			break;
		}

		const std::string_view itemNameView(itemData->GetName());
		message.replace(posBegin, posEnd - posBegin + 1, itemNameView);
		startPos = posBegin + itemNameView.size();
	}
}

void PythonLocaleManager::ReplaceGeneralLocaleStrings(std::string& message) const
{
	constexpr std::string_view localeTokenBegin = "[LS;";
	size_t startPos = 0;

	while (true)
	{
		const size_t posBegin = message.rfind(localeTokenBegin, startPos);
		if (posBegin == std::string::npos)
		{
			break;
		}

		const size_t posMid = message.find(';', posBegin);
		if (posMid == std::string::npos)
		{
			break;
		}

		size_t posEnd = message.find(']', posMid + 1);
		if (posEnd == std::string::npos)
		{
			break;
		}

		size_t posInner = message.find('[', posMid + 1);
		if (posInner != std::string::npos && posInner < posEnd)
		{
			posEnd = message.find(']', posEnd + 1);
		}

		std::string args = message.substr(posMid + 1, posEnd - posMid - 1);
		CTokenVector tokenVector;
		SplitLine(args.c_str(), ";", &tokenVector);

		if (tokenVector.empty())
		{
			break;
		}

		auto it = localeStringMaps[static_cast<int>(LocaleStringType::GENERAL)].find(tokenVector[0]);
		if (it == localeStringMaps[static_cast<int>(LocaleStringType::GENERAL)].end())
		{
			TraceError("CPythonLocale::ReplaceLocaleString wrong LS vnum : %s", tokenVector[0].c_str());
			break;
		}

		if (tokenVector.size() > 1)
		{
			try
			{
				boost::format f(it->second);
				for (size_t i = 1; i < tokenVector.size(); ++i)
				{
					f% tokenVector[i];
				}
				message.replace(posBegin, posEnd - posBegin + 1, f.str());
			}
			catch (const std::exception& ex)
			{
				TraceError("PythonLocaleManager::ReplaceGeneralLocaleStrings: Error: %s", ex.what());
				break;
			}
		}
		else
		{
			message.replace(posBegin, posEnd - posBegin + 1, it->second);
		}
	}
}

void PythonLocaleManager::ReplaceQuestLocaleStrings(std::string& message) const
{
	while (true)
	{
		size_t posBegin = message.rfind("[LC;");
		if (posBegin == std::string::npos)
			break;

		size_t posMid = message.find(';', posBegin);
		if (posMid == std::string::npos)
			break;

		++posMid;

		size_t posInner = message.find('[', posMid);
		size_t posEnd = message.find(']', posMid);
		if (posEnd == std::string::npos)
			break;

		if (posInner != std::string::npos && posInner < posEnd)
			posEnd = message.find(']', posEnd + 1);

		std::string args = message.substr(posMid, posEnd - posMid);

		CTokenVector tokenVector;
		SplitLine(args.c_str(), ";", &tokenVector);

		if (tokenVector.empty())
			break;

		auto it = localeStringMaps[static_cast<int>(LocaleStringType::QUEST)].find(tokenVector[0]);
		if (it == localeStringMaps[static_cast<int>(LocaleStringType::QUEST)].end())
		{
			TraceError("CPythonLocale::ReplaceLocaleString wrong LC vnum : %s", tokenVector[0].c_str());
			break;
		}

		if (tokenVector.size() > 1)
		{
			try
			{
				boost::format f(it->second);
				for (size_t i = 1; i < tokenVector.size(); ++i)
				{
					f% tokenVector[i];
				}
				message.replace(posBegin, posEnd - posBegin + 1, f.str());
			}
			catch (const std::exception& ex)
			{
				TraceError("PythonLocaleManager::ReplaceGeneralLocaleStrings: Error: %s", ex.what());
				break;
			}
		}
		else
		{
			message.replace(posBegin, posEnd - posBegin + 1, it->second);
		}
	}
}

void PythonLocaleManager::ReplaceOXQuizLocaleStrings(std::string& message) const
{
	constexpr std::string_view oxQuizTokenBegin = "[LOX;";
	size_t startPos = 0;

	while (true)
	{
		const size_t posBegin = message.rfind(oxQuizTokenBegin, startPos);
		if (posBegin == std::string::npos)
		{
			break;
		}

		const size_t posMid = message.find(';', posBegin);
		if (posMid == std::string::npos)
		{
			break;
		}

		size_t posEnd = message.find(']', posMid + 1);
		if (posEnd == std::string::npos)
		{
			break;
		}

		size_t posInner = message.find('[', posMid + 1);
		if (posInner != std::string::npos && posInner < posEnd)
		{
			posEnd = message.find(']', posEnd + 1);
		}

		std::string args = message.substr(posMid + 1, posEnd - posMid - 1);
		CTokenVector tokenVector;
		SplitLine(args.c_str(), ";", &tokenVector);

		if (tokenVector.empty())
		{
			break;
		}
		auto it = localeStringMaps[static_cast<int>(LocaleStringType::OXQUIZ)].find(tokenVector[0]);
		if (it == localeStringMaps[static_cast<int>(LocaleStringType::OXQUIZ)].end())
		{
			TraceError("CPythonLocale::ReplaceOXQuizLocaleString wrong LOX level-key : %s", tokenVector[0].c_str());
			break;
		}

		if (tokenVector.size() > 1)
		{
			try
			{
				boost::format f(it->second);
				for (size_t i = 1; i < tokenVector.size(); ++i)
				{
					f% tokenVector[i];
				}

				message.replace(posBegin, posEnd - posBegin + 1, f.str());
			}
			catch (const std::exception& ex)
			{
				TraceError("CPythonLocale::ReplaceOXQuizLocaleString: Error: %s", ex.what());
				break;
			}
		}
		else
		{
			message.replace(posBegin, posEnd - posBegin + 1, it->second);
		}
	}
}