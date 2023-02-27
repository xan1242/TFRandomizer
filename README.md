# Yu-Gi-Oh! Tag Force Randomizer

This is a command-line based tool which randomizes:

- deck recipes

- shop boxes

More ideas on what to randomize are welcome! If you have any ideas, open an issue.

## Compatibility

This tool should be game agnostic as long as you provide the correct parameters.

Currently this was only tested with Tag Force 3. More games (and their parameters) are to come!

## Usage

This is a command-line based version of the tool. Currently it is a very involved process to get results. A GUI wrapper is planned as well, but in the meantime, this is how you use it.

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

Currently, only Tag Force 3 (ULES01183) is known. More is to come.



Tag Force 3 offsets:

```
TF3 offsets:
BoxInfoOffset = @file 0x20324, @mem 0x202D0
SegmentOffset = @file - @mem = 0x54
BoxCount = 48
```

(todo - insert a table here when there's more available)



## Using randomized recipes

Currently there is no easy way to utilize these other than by hex editing the data in.

A modification to the EBOOT is required in order to redirect the embedded files to files on disc. This is necessary in case the EHP file is bigger than what it fits to the EBOOT.

As of now I've only written the modification for Tag Force 3. This will be made public once the GUI tool is made as well. This will probably be written as a patcher option either integrated into TFRandomizer or made separately.


