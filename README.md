# Ruggedised Image-Colour-Transfer
============================================================

##### A Ruggedised Implementation of the Colour Transfer Method Proposed by Xiao and Lizhuang.

Reference

Xiao, Xuezhong, and Lizhuang Ma. "Color Transfer in Correlated Color Space." In Proceedings of the 2006 ACM international conference on Virtual reality continuum and its applications, pp. 305-309. ACM, 2006.

This incorporates additional code written and devised by T E Johnson. It enables an improved output image for input image pairs where the original processing method gave an unsatisfactory output.

This is a C++/OpenCV implementation of the processing that was first presented as a Matlab implementation here.
https://github.com/hangong/Xiao06_color_transfer.
The method is described further in the comments section of the function 'MatchColumns' and also here.  https://github.com/hangong/Xiao06_color_transfer/issues/1 


#  
#  

![Composite of Flower Image: Inputs and Outputs](Documents/Images/Composite.jpg?raw=true)

#  
#  

The Xiao method has received particular attention because it is referenced in the survey of colour transfer methods by Faridul et al.  (https://www.researchgate.net/publication/262637346_A_Survey_of_Color_Mapping_and_its_Applications)  

However an alternative method proposed by Pitie appears to offer reliable processing without any need for further ruggedisation and appears to offer comparable if not superior results to those achieved by the ‘Ruggedised Xiao’ method.  

See https://github.com/frcs   
And https://github.com/TJCoding/Pitie_Image_Colour_Transfer



