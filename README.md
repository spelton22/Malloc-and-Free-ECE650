Overview of my implementation of the allocation policies: 

For my first fit malloc algorithm, I first checked to see if I had any free regions, whether it be the first time calling a malloc or there were no more free spaces in my heap. I used 4092 as my incremental size but also checked if the size of requested memory was larger than that and allocated the size requested * 2. Once I knew I had some free space, I looped through the free regions and checked if the free region was big enough to hold the requested memory. If I got to the end of my list and didn’t find a suitable free region, I then allocated more memory at the end based on the size of the requested malloc. I then checked if I needed to combine the last free region with the newly allocated memory. (These steps were repeated in every check) I then created a used space block and set the return pointer and if there was free space left over in that block that was able to hold my metadata I created a new free node. If there was not enough free space to hold my metadata I kept that memory region with the allocation block. Then I checked if the free region found was the first free region, since I would need to update it, and repeated the steps listed above. Then I checked if the free region was not equal to the first free block, and repeated the steps listed above.
These steps were the same for my best fit malloc algorithm, except for finding the free region to hold the requested size. I looped through all my free regions and found the region with the smallest leftover space.
For my free algorithm, first fit and best fit used the same code, I first created a new node at the location to be added. I then checked if that node was before my first free node, and then checked if they were adjacent regions. I then updated the pointers and lengths as needed. If my region to be freed was not before my first free pointer, I looped through all my free regions until I found the free regions before and after the new free node. I then checked if the new free node had adjacent free regions and combined when necessary. I then had a last check to see if the node being freed was after all my free regions and checked if it has adjacent free regions to combine with.
