# Yu-Gi-Oh! Tag Force Randomizer

This is a command-line based tool which randomizes:

- deck recipes

- shop boxes

More ideas on what to randomize are welcome! If you have any ideas, open an issue.

## Compatibility

This tool should be game agnostic as long as you provide the correct parameters.

Currently this was only tested with Tag Force 3 EU and Tag Force 5 US. More games (and their parameters) are to come!

## Usage

This is a command-line based version of the tool. Currently it is a very involved process to get results. 

A GUI wrapper is available [here](https://github.com/xan1242/TFRandomizerGUI), but if you wish to use this manually, this is how you do it:

## Data preparation

1. Extract EBOOT.BIN, the shop script and the card database (cardinfo) out of the ISO (EBOOT is in SYSDIR, shop is in USRDIR/gmodule, card database is in USRDIR/duelsys)

2. Extract embedded EHPs out of the EBOOT using a tool like [EHPScanner](https://github.com/xan1242/EHPScanner)

3. Find the EHP containing recipes (should have a lot of files with RCPxxx.ydc)

4. Extract both the recipe EHP and cardinfo EHP (by using a tool like [ehppack](https://github.com/xan1242/ehppack))

5. Get the card ID list with [TFCardEdit](https://github.com/xan1242/TFCardEdit) `TFCardEdit -l cardinfo_eng MinCardID MaxCardID cardlist.ini` (where min/max card IDs match the ones for your game)

Once that's done, you can move on to using the randomizer.

## Using TFRandomizer - deck recipe shuffling/randomization

Basic usage to shuffle:

`TFRandomizer -rs RecipeFolder OutFolder IncludePlayerRecipe [RandomizerSeed]`

Basic usage to randomize: 

`TFRandomizer -rr RecipeFolder OutFolder IncludePlayerRecipe [RandomizerSeed]`

Both do very similar things. The difference is that shuffler will not duplicate decks, whereas randomizer may do it.

- RecipeFolder: the folder of the recipe EHP extracted by the ehppack tool

- OutFolder: the output folder where the shuffled recipes will be placed

- IncludePlayerRecipe: a boolean, 0 or 1 value. If enabled, the player's starting deck will also be affected (RCP000.ydc)

- RandomizerSeed: optional value, for setting the seed. If omitted, the seed will be derived from the time of execution.

## Using TFRandomizer - shop randomizer

Basic usage:

`TFRandomizer -s CardIDList InShopPrx BoxInfoOffset SegmentOffset BoxCount MinPackPrice MaxPackPrice MinPackSize MaxPackSize OutShopPrx [RandomizerSeed]`

This command requires a lot of information to function properly.

- CardIDList: the cardlist ini file that was generated with TFCardEdit before

- InShopPrx: the shop prx file you got out of the ISO

- BoxInfoOffset: offset of box info in the file (Tag Force 3 (ULES01183) example: 0x20324)

- SegmentOffset: offset of the memory segment from the file start (Tag Force 3 (ULES01183) example: 0x54)

- BoxCount: number of boxes in the shop

- Min/MaxPackPrice: range of min/max prices of packs

- Min/MaxPackSize: range of min/max size of box (TODO: correct the name to reflect the real state)

- OutShopPrx: output file where the modified shop prx will be stored

- RandomizerSeed: same as before, for setting the seed, optional value

## Shop script offsets

You can find the shop data manually by searching it in a tool like IDA.

Here are the currently known ones:

| Game                   | BoxInfoOffset (file) | BoxInfoOffset (mem) | SegmentOffset | BoxCount |
| ---------------------- | -------------------- | ------------------- | ------------- | -------- |
| Tag Force 3 ULES-01183 | 0x2031C              | 0x202C8             | 0x54          | 48       |
| Tag Force 5 ULUS-10555 | 0x23090              | 0x2303C             | 0x54          | 60       |

## Using randomized recipes

Currently there is no easy way to utilize these other than by hex editing the data in.

A modification to the EBOOT is required in order to redirect the embedded files to files on disc. This is necessary in case the EHP file is bigger than what it fits to the EBOOT.

As of now I've only written the modifications for Tag Force 3 EU and Tag Force 5 US.



The modified EBOOTs can be found here:

- Tag Force 3 EU: https://github.com/xan1242/TFRandomizerGUI/tree/master/TF3_EU

- Tag Force 5 US: https://github.com/xan1242/TFRandomizerGUI/tree/master/TF5_US



The current plan is to write a PRX plugin which will do these modifications during runtime and hopefully simplfy the process!


