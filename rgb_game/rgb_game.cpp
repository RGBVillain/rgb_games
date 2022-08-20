#include <cstdlib>
#include <cstdint>
#include <vector>
#include <string>

#define superp(format, ...) printf(format "\n", __VA_ARGS__)


struct SJump {
	::std::string					Text;
	uint32_t						Jump;
};

struct SPage {
	::std::vector<::std::string>	TextLines;
	::std::vector<SJump>			PageJumps;
};

struct SGame {
	::std::vector<SPage>			Pages				= {};
	uint32_t						CurrentPage			= 0;

	char							StoryFolder[4096]	= "test_story";
};

int						fileSize		(FILE* fp) {
	uint32_t				position		= (uint32_t)ftell(fp);
	fseek(fp, 0, SEEK_END);
	uint32_t				lastPosition	= (uint32_t)ftell(fp);
	fseek(fp, position, SEEK_SET);
	return (int)lastPosition;
}

int						splitLines		(const ::std::string & pageBytes, ::std::vector<std::string> & pageLines) {
	pageLines.clear();
	uint32_t				lastOffset		= 0;
	for(uint32_t iOffset = 0; iOffset < pageBytes.size(); ++iOffset) {
		if(pageBytes[iOffset] == '\n') {
			const ::std::string	lineToPush		= pageBytes.substr(lastOffset, iOffset - lastOffset + 1);
			pageLines.push_back(lineToPush);
			lastOffset		= ++iOffset;
		}
	}
	if(pageBytes.size() > lastOffset) {
		pageLines.push_back(pageBytes.substr(lastOffset) + "\n");
	}
	return 0;
}

template <typename _tCell>
int32_t					reverse			(_tCell* elements, uint32_t elementCount)	 {
	for(uint32_t i = 0, swapCount = elementCount / 2; i < swapCount; ++i) {
		_tCell									old			= elements[i];
		elements[i]							= elements[elementCount - 1 - i];
		elements[elementCount - 1 - i]	= old;
	}
	return 0;
}

int						loadLines		(SPage & page, const ::std::vector<std::string> & pageLines) {
	page.TextLines			= pageLines;
	page.PageJumps			= {};
	for(int32_t iLine = (int32_t)page.TextLines.size() - 1; iLine >= 0; --iLine) {
		if(page.TextLines[iLine].size() <= 2 && 0 == page.PageJumps.size()) {
			page.TextLines.pop_back();
			continue;
		}

		uint32_t			jumpIndex			= atoi(page.TextLines[iLine].data());
		if(0 == jumpIndex && page.TextLines[iLine][0] != '0') {
			break;
		}
		page.PageJumps.push_back({page.TextLines[iLine], jumpIndex});
		page.TextLines.pop_back();
	}

	::reverse(&page.PageJumps[0], (uint32_t)page.PageJumps.size());
	return 0;
}

int						loadPage		(const char* folderName, SPage & page, uint32_t pageIndex) {
	char					fileName[4096]	= {};
	sprintf_s(fileName, "%s/%u.txt", folderName, pageIndex);
	FILE		* fp			= 0;
	if(fopen_s(&fp, fileName, "rb")) {
		superp("Failed to open file '%s'", fileName);
		return 1;
	}

	int						textSize		= fileSize(fp);
	::std::string			pageBytes;
	pageBytes.resize(textSize);
	fread(&pageBytes[0], 1, textSize, fp);
	fclose(fp);	
	::std::vector<::std::string> pageLines;
	::splitLines(pageBytes, pageLines);
	::loadLines(page, pageLines);


	return 0;
}

int						validJump	(const ::std::vector<SJump> & jumps, uint32_t indexToTest) {
	for(uint32_t iJump = 0; iJump < jumps.size(); ++iJump) {
		if(jumps[iJump].Jump == indexToTest) {
			return iJump;
		}
	}
	return -1;
}

int						handleInput			(SGame & game) {
	constexpr uint32_t		BUFFER_SIZE			= 4096;
	char					buffer[BUFFER_SIZE]	= {};

	
	while(fgets(buffer, BUFFER_SIZE, stdin)) {
		if(0 == strncmp(buffer, "exit", 4)) {
			superp("*** You close the book ***");
			return 1;
		}
		const int32_t			pageIndex		= atoi(buffer);
		if(pageIndex == 0 && buffer[0] != '0') {
	        superp("Come again?");
			continue;
		}

		SPage					& page			= game.Pages[game.CurrentPage];
		int32_t					jumpIndex		= validJump(page.PageJumps, pageIndex);
		if(jumpIndex < 0) {
	        superp("Come again?");
			continue;
		}
		game.CurrentPage	= page.PageJumps[jumpIndex].Jump;
		break;
    }
    return 0;
}


int					drawPage				(SPage & page) {
	for(uint32_t iLine = 0; iLine < page.TextLines.size(); ++iLine) {
		printf("%s", page.TextLines[iLine].data());
	}
	superp("");
	for(uint32_t iJump = 0; iJump < page.PageJumps.size(); ++iJump) {
		printf("%s", page.PageJumps[iJump].Text.data());
	}
	return 0;
}


int					step					(SGame & game) {
	if(game.Pages.size() <= game.CurrentPage)
		game.Pages.resize(game.CurrentPage + 1);

	SPage					& page			= game.Pages[game.CurrentPage];

	if(loadPage(game.StoryFolder, page, game.CurrentPage)) {
		superp("Failed to load page %i", game.CurrentPage);
		return 1;
	}

	drawPage(page);

	if(handleInput(game)) {
		return 1;
	}

	system("cls");
	return 0;
}

int main() {

	SGame				game;
	while(true) {
		if(step(game))
			break;
	}
}