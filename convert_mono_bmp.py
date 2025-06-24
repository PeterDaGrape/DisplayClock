from PIL import Image
import PIL
image = Image.open("/Users/petervine/Downloads/classic.png")

image = image.resize((128, 128), Image.LANCZOS)  # Resize to 128x128 with high-quality filter
image = image.convert('1', dither=Image.NONE) # convert image to black and white
inverted_image = PIL.ImageOps.invert(image)
image.save('result.bmp', 'BMP')