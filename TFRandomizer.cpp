// Tag Force Randomizer
// by Xan/Tenjoin
// 
// Tool can randomize: 
// - deck recipes
// - shop boxes
// 
// This tool should be game agnostic as long as you provide the correct parameters.
// You can find the shop data manually by searching it in a tool like IDA.
// 
// TF3 offsets:
// BoxInfoOffset = @file 0x20324, @mem 0x202D0
// SegmentOffset = @file - @mem = 0x54
// BoxCount = 48
// 



#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <filesystem>
#include <chrono>
#include <sys/stat.h>
#include "includes/mINI/src/mini/ini.h"

#if defined (_WIN32) || defined (_WIN64)
#define path_separator "\\"
#define path_separator_char '\\'
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <strsafe.h>
#endif

#if __GNUC__
#include <dirent.h>
#define path_separator "/"
#define path_separator_char '/'
#endif

#if _WIN64
#define pcast int64_t
#else
#if __GNUC__
#if __x86_64__ || __ppc64__ || __MINGW64__ || __aarch64__
#define pcast int64_t
#endif
#else
#define pcast int32_t
#endif
#define pcast int32_t
#endif

using namespace std;
namespace fs = std::filesystem;

struct PackInfo
{
    uint32_t commonCardIDsPointer;
    uint32_t commonCardCount;
    uint32_t superRareCardIDsPointer;
    uint32_t superRareCardCount;
    uint32_t ultraRareCardIDsPointer;
    uint32_t ultraRareCardCount;
    uint32_t ultimateRareCardIDsPointer;
    uint32_t ultimateRareCardCount;
};

struct BoxInfo
{
    uint32_t packPointer;
    uint16_t packCount;
    uint16_t unk1;
    uint16_t packPrice;
    uint16_t unk2;
};

vector<string> FileDirectoryListing;
vector<string> FileDirectoryListingRandomized;
vector<string> FileDirectoryListingOther;

// box randomize stuff
vector<uint16_t> CardIDList;
vector<BoxInfo*> ShopBoxes;
void* ShopPrxMem;
int MinPackPrice = 0;
int MaxPackPrice = 100;
int MinPackCount = 1;
int MaxPackCount = 20;

unsigned long seed;

int bRandom_Custom(int range)
{
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> distrib(0, range);
    int result = distrib(gen);
    seed = gen() ^ 0x1D872B41;

    return result;
}

int bRandom_MinMax(int rangemin, int rangemax)
{
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> distrib(rangemin, rangemax);
    int result = distrib(gen);
    seed = gen() ^ 0x1D872B41;

    return result;
}

#ifdef WIN32
DWORD GetDirectoryListing(const char* FolderPath)
{
    WIN32_FIND_DATA ffd = { 0 };
    TCHAR  szDir[MAX_PATH];
    char MBFilename[MAX_PATH];
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwError = 0;
    unsigned int NameCounter = 0;

    mbstowcs(szDir, FolderPath, MAX_PATH);
    StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

    if (strlen(FolderPath) > (MAX_PATH - 3))
    {
        printf("Directory path is too long.\n");
        return -1;
    }

    hFind = FindFirstFile(szDir, &ffd);

    if (INVALID_HANDLE_VALUE == hFind)
    {
        printf("FindFirstFile error\n");
        return dwError;
    }

    do
    {
        if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            wcstombs(MBFilename, ffd.cFileName, MAX_PATH);
            string rcp(MBFilename);
            if ((rcp.find("RCP") != string::npos))
            {
                FileDirectoryListing.push_back(MBFilename);
            }
            else
                FileDirectoryListingOther.push_back(MBFilename);
        }
    } while (FindNextFile(hFind, &ffd) != 0);

    dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
    {
        printf("FindFirstFile error\n");
    }
    FindClose(hFind);

    return dwError;
}
#elif __GNUC__
void GetDirectoryListing(const char* FolderPath)
{
    struct dirent* dp;
    DIR* dir = opendir(FolderPath);
    unsigned int NameCounter = 0;

    while ((dp = readdir(dir)))
    {
        // ignore the current and previous dir files...
        if (!((strcmp(dp->d_name, ".") == 0) || (strcmp(dp->d_name, "..") == 0)))
        {
            string rcp(dp->d_name));
            if ((rcp.find("RCP") != string::npos))
                FileDirectoryListing.push_back(dp->d_name));
            else
                FileDirectoryListingOther.push_back(dp->d_name));
        }
    }
    closedir(dir);
}
#endif

int ShuffleRecipes(const char* inFolder, const char* outFolder, bool bIncludePlayerRecipe)
{
    std::cout << "Reading recipe directory...\n";
#ifdef WIN32
    DWORD dirlistresult = GetDirectoryListing(inFolder);
    if (!((dirlistresult == ERROR_SUCCESS) || (dirlistresult == ERROR_NO_MORE_FILES)))
    {
        std::cout << "ERROR: Failed getting the directory listing. Error code: 0x" <<  std::hex << dirlistresult << '\n';
        return -1;
    }
#elif
    GetDirectoryListing(inFolder);
#endif
    std::cout << "Shuffling deck recipes...\n";
    vector<string>::iterator ptr = FileDirectoryListing.begin();

    if (!bIncludePlayerRecipe)
        advance(ptr, 1);

    shuffle(ptr, FileDirectoryListing.end(), std::default_random_engine(seed));

    std::cout << "Copying shuffled files...\n";
    if (!(fs::exists(outFolder) && fs::is_directory(outFolder)))
    {
        std::cout << "Creating directory: " << outFolder << "\n";
        fs::create_directory(outFolder);
    }

    // recipe counter
    int rC = 0;
    char recipeStr[16];

    for (string i : FileDirectoryListing)
    {
        sprintf_s(recipeStr, "RCP%03d.ydc", rC);

        string inpath = inFolder;
        inpath += path_separator;
        inpath += i;

        string outpath = outFolder;
        outpath += path_separator;
        outpath += recipeStr;

        try
        {
            fs::copy(inpath, outpath, fs::copy_options::overwrite_existing);
        }
        catch (fs::filesystem_error err)
        {
            std::cout << "ERROR: Can't copy: [" << inpath << "] to [" << outpath << "]\n";
            std::cout << "Exception: " << err.what() << "\n";
            return -1;
        }
        rC++;
    }

    for (string i : FileDirectoryListingOther)
    {
        string inpath = inFolder;
        inpath += path_separator;
        inpath += i;

        string outpath = outFolder;
        outpath += path_separator;
        outpath += i;

        try
        {
            fs::copy(inpath, outpath, fs::copy_options::overwrite_existing);
        }
        catch (fs::filesystem_error err)
        {
            std::cout << "ERROR: Can't copy: [" << inpath << "] to [" << outpath << "]\n";
            std::cout << "Exception: " << err.what() << "\n";
            return -1;
        }
    }

    return 0;
}

int RandomizeRecipes(const char* inFolder, const char* outFolder, bool bIncludePlayerRecipe)
{
    std::cout << "Reading recipe directory...\n";
#ifdef WIN32
    DWORD dirlistresult = GetDirectoryListing(inFolder);
    if (!((dirlistresult == ERROR_SUCCESS) || (dirlistresult == ERROR_NO_MORE_FILES)))
    {
        std::cout << "ERROR: Failed getting the directory listing. Error code: 0x" << std::hex << dirlistresult << '\n';
        return -1;
    }
#elif
    GetDirectoryListing(inFolder);
#endif
    std::cout << "Randomizing deck recipes...\n";

    int j = 0;
    if (!bIncludePlayerRecipe)
        j = 1;

    for (; j < FileDirectoryListing.size(); j++)
    {
        FileDirectoryListingRandomized.push_back(FileDirectoryListing.at(bRandom_Custom(FileDirectoryListing.size()) % FileDirectoryListing.size()));
    }

    std::cout << "Copying randomized files...\n";
    if (!(fs::exists(outFolder) && fs::is_directory(outFolder)))
    {
        std::cout << "Creating directory: " << outFolder << "\n";
        fs::create_directory(outFolder);
    }

    // recipe counter
    int rC = 0;
    if (!bIncludePlayerRecipe)
        rC = 1;
    // recipe string
    char recipeStr[16];

    if (!bIncludePlayerRecipe)
    {
        string inpath = inFolder;
        inpath += path_separator;
        inpath += "RCP000.ydc";

        string outpath = outFolder;
        outpath += path_separator;
        outpath += "RCP000.ydc";

        try
        {
            fs::copy(inpath, outpath, fs::copy_options::overwrite_existing);
        }
        catch (fs::filesystem_error err)
        {
            std::cout << "ERROR: Can't copy: [" << inpath << "] to [" << outpath << "]\n";
            std::cout << "Exception: " << err.what() << "\n";
            return -1;
        }
    }

    for (string i : FileDirectoryListingRandomized)
    {
        sprintf_s(recipeStr, "RCP%03d.ydc", rC);

        string inpath = inFolder;
        inpath += path_separator;
        inpath += i;

        string outpath = outFolder;
        outpath += path_separator;
        outpath += recipeStr;

        try
        {
            fs::copy(inpath, outpath, fs::copy_options::overwrite_existing);
        }
        catch (fs::filesystem_error err)
        {
            std::cout << "ERROR: Can't copy: [" << inpath << "] to [" << outpath << "]\n";
            std::cout << "Exception: " << err.what() << "\n";
            return -1;
        }
        rC++;
    }

    for (string i : FileDirectoryListingOther)
    {
        string inpath = inFolder;
        inpath += path_separator;
        inpath += i;

        string outpath = outFolder;
        outpath += path_separator;
        outpath += i;

        try
        {
            fs::copy(inpath, outpath, fs::copy_options::overwrite_existing);
        }
        catch (fs::filesystem_error err)
        {
            std::cout << "ERROR: Can't copy: [" << inpath << "] to [" << outpath << "]\n";
            std::cout << "Exception: " << err.what() << "\n";
            return -1;
        }
    }

    return 0;
}

int LoadCardIDList(const char* Filename)
{
    std::cout << "Loading CardID list...\n";

    if (!fs::exists(Filename))
    {
        std::cout << "ERROR: Can't open file for reading: " << Filename << "\n";
        return -1;
    }
    mINI::INIFile inifile(Filename);
    mINI::INIStructure ini;
    inifile.read(ini);

    // iterate through keys and get them as card IDs
    if (ini.has("IDPasswords"))
    {
        for (auto const& i : ini["IDPasswords"])
        {
            CardIDList.push_back((stoul(i.first)) & 0xFFFF);
        }
    }
    else
    {
        std::cout << "ERROR: File does not contain a valid \"IDPasswords\" section!\n";
        return -1;
    }

    return 0;
}

// inShopScript = input shop EhScript, example: rel_shop.prx
// outShopScript = output shop file
// BoxInfoOffset = offset of box info in the file (Tag Force 3 (ULES01183) example: 0x20324)
// SegmentOffset = offset of the memory segment from the file start (Tag Force 3 (ULES01183) example: 0x54)
// BoxCount = number of boxes in the script
int RandomizePacks(const char* inShopScript, const char* outShopScript, off_t BoxInfoOffset, off_t SegmentOffset, int BoxCount)
{
    std::cout << "Opening file: " << inShopScript << "\n";

    FILE* fin = fopen(inShopScript, "rb");
    if (!fin)
    {
        std::cout << "ERROR: Can't open file for reading: " << inShopScript << "\n";
        perror("ERROR");
        return -1;
    }

    std::cout << "Loading Shop EhScript to memory...\n";

    // load the PRX to memory
    size_t shopsize = fs::file_size(inShopScript);
    ShopPrxMem = malloc(shopsize);
    if (!ShopPrxMem)
    {
        std::cout << "Error allocating memory.\n";
        return -1;
    }
    fread(ShopPrxMem, shopsize, 1, fin);
    fclose(fin);

    for (int i = 0; i < BoxCount; i++)
    {
        BoxInfo* boxptr = (BoxInfo*)((pcast)ShopPrxMem + BoxInfoOffset + sizeof(BoxInfo) * i);
        ShopBoxes.push_back(boxptr);
    }

    std::cout << "Randomizing shop data...\n";

    for (BoxInfo* i : ShopBoxes)
    {
        if ((MinPackCount > 0) && (MaxPackCount > 0))
        {
            if (MaxPackCount < MinPackCount)
                MaxPackCount = MinPackCount;

            MinPackCount &= 0xFFFF;
            MaxPackCount &= 0xFFFF;

            i->packCount = bRandom_MinMax(MinPackCount, MaxPackCount) & 0xFFFF;
        }

        if ((MinPackPrice >= 0) && (MaxPackPrice >= 0))
        {
            MinPackPrice &= 0xFFFF;
            MaxPackPrice &= 0xFFFF;

            i->packPrice = bRandom_MinMax(MinPackPrice, MaxPackPrice) & 0xFFFF;
        }

        // we do NOT modify the pointers - they stay clean! copy the pointer first, edit it and then dereference!
        PackInfo* pi = (PackInfo*)i->packPointer;
        pi = (PackInfo*)(pcast)((pcast)pi + (pcast)ShopPrxMem + SegmentOffset);

        uint16_t* cardIDs;

        cardIDs = (uint16_t*)pi->commonCardIDsPointer;
        cardIDs = (uint16_t*)(pcast)((pcast)cardIDs + (pcast)ShopPrxMem + SegmentOffset);
        for (int j = 0; j < pi->commonCardCount; j++)
        {
            cardIDs[j] = CardIDList.at(bRandom_Custom(CardIDList.size() - 1));
        }

        cardIDs = (uint16_t*)pi->superRareCardIDsPointer;
        cardIDs = (uint16_t*)(pcast)((pcast)cardIDs + (pcast)ShopPrxMem + SegmentOffset);
        for (int j = 0; j < pi->superRareCardCount; j++)
        {
            cardIDs[j] = CardIDList.at(bRandom_Custom(CardIDList.size() - 1));
        }

        cardIDs = (uint16_t*)pi->ultraRareCardIDsPointer;
        cardIDs = (uint16_t*)(pcast)((pcast)cardIDs + (pcast)ShopPrxMem + SegmentOffset);
        for (int j = 0; j < pi->ultraRareCardCount; j++)
        {
            cardIDs[j] = CardIDList.at(bRandom_Custom(CardIDList.size() - 1));
        }

        cardIDs = (uint16_t*)pi->ultimateRareCardIDsPointer;
        cardIDs = (uint16_t*)(pcast)((pcast)cardIDs + (pcast)ShopPrxMem + SegmentOffset);
        for (int j = 0; j < pi->ultimateRareCardCount; j++)
        {
            cardIDs[j] = CardIDList.at(bRandom_Custom(CardIDList.size() - 1));
        }
    }

    std::cout << "Writing output file: " << outShopScript << "\n";

    FILE* fout = fopen(outShopScript, "wb");
    if (!fout)
    {
        std::cout << "ERROR: Can't open file for writing: " << outShopScript << "\n";
        perror("ERROR");
        return -1;
    }

    fwrite(ShopPrxMem, shopsize, 1, fout);
    fclose(fout);
    free(ShopPrxMem);

    return 0;
}

void ShowHelp(char* firstarg)
{
    std::cout << "USAGE (shuffle recipes): " << firstarg << " -rs RecipeFolder OutFolder IncludePlayerRecipe [RandomizerSeed]\n";
    std::cout << "USAGE (randomize recipes): " << firstarg << " -rr RecipeFolder OutFolder IncludePlayerRecipe [RandomizerSeed]\n";
    std::cout << "USAGE (randomize shop): " << firstarg << " -s CardIDList InShopPrx BoxInfoOffset SegmentOffset BoxCount MinPackPrice MaxPackPrice MinPackSize MaxPackSize OutShopPrx [RandomizerSeed]\n";
    std::cout << "\n";
    std::cout << "Shuffled recipes do not contain duplicates, while randomized may contain duplicated recipes.\n";
    std::cout << "IncludePlayerRecipe can either be 0 or 1. If enabled, player's starter deck will also be randomized.\n";
    std::cout << "If the min/max prices are set below 0, they will not be randomized.\n";
    std::cout << "If the min/max sizes are set equal or below 0, they will not be randomized.\n";
    std::cout << "Minimum size/price cannot be larger than the maximum\n";
    std::cout << "RandomizerSeed has to be a number, which is optional. If omitted, seed will be set from the current time of execution.\n";
}

int main(int argc, char* argv[])
{
    std::cout << "Yu-Gi-Oh! Tag Force Randomizer\n";
    
    if (argc <= 4)
    {
        ShowHelp(argv[0]);
        return -1;
    }

    if (argv[1][0] == '-')
    {
        if ((argv[1][1] == 'r'))
        {
            if (argc <= 5)
            {
                seed = std::chrono::system_clock::now().time_since_epoch().count();
            }
            else
            {
                try
                {
                    seed = stol(argv[5]);
                }
                catch (std::invalid_argument& ex)
                {
                    std::cout << "Invalid RandomizerSeed argument: " << ex.what() << "\n";
                }
                catch (std::exception& ex)
                {
                    std::cout << "Error setting RandomizerSeed, exception: " << ex.what() << "\n";
                }
            }

            std::cout << "Randomizer seed is: " << seed << "\n";

            bool bPlayerRecipe = false;
            try
            {
                bPlayerRecipe = stol(argv[4]) != 0;
            }
            catch (std::invalid_argument& ex)
            {
                std::cout << "Invalid IncludePlayerRecipe argument: " << ex.what() << "\n";
            }
            catch (std::exception& ex)
            {
                std::cout << "Error setting IncludePlayerRecipe, exception: " << ex.what() << "\n";
            }


            if ((argv[1][2] == 's'))
            {
                ShuffleRecipes(argv[2], argv[3], bPlayerRecipe);
                return 0;
            }
            if ((argv[1][2] == 'r'))
            {
                RandomizeRecipes(argv[2], argv[3], bPlayerRecipe);
                return 0;
            }
        }
        if ((argv[1][1] == 's'))
        {
            if (argc <= 12)
            {
                seed = std::chrono::system_clock::now().time_since_epoch().count();
            }
            else
            {
                try
                {
                    seed = stol(argv[12]);
                }
                catch (std::invalid_argument& ex)
                {
                    std::cout << "Invalid RandomizerSeed argument: " << ex.what() << "\n";
                }
                catch (std::exception& ex)
                {
                    std::cout << "Error setting RandomizerSeed, exception: " << ex.what() << "\n";
                }
            }

            std::cout << "Randomizer seed is: " << seed << "\n";

            LoadCardIDList(argv[2]);

            MinPackPrice = stol(argv[7]);
            MaxPackPrice = stol(argv[8]);
            MinPackCount = stol(argv[9]);
            MaxPackCount = stol(argv[10]);

            RandomizePacks(argv[3], argv[11], stoul(argv[4], nullptr, 16), stoul(argv[5], nullptr, 16), stoul(argv[6]));

            return 0;
        }
    }

    ShowHelp(argv[0]);
    return -1;
}
