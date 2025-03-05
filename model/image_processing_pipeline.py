import cv2
import numpy as np

# Load image
image_path = "../data/720p_test_8.jpg"
image = cv2.imread(image_path, cv2.IMREAD_GRAYSCALE)

# Step 1: Initial Thresholding for Contrast Boost
threshold_value = 40  
_, thresholded_image = cv2.threshold(image, threshold_value, 255, cv2.THRESH_BINARY)
cv2.imwrite("../data/test_8_contrast_boosted.png", thresholded_image)

# Step 2: Downsample to 28x28
downsampled_image_area = cv2.resize(thresholded_image, (28, 28), interpolation=cv2.INTER_AREA)
cv2.imwrite("../data/test_8_downsampled_28x28.png", downsampled_image_area)

# Step 2: Downsample to 28x28 using manual INTER_AREA
def inter_area_downsample(image, new_width, new_height):
    """
    Manually downsamples an image using INTER_AREA interpolation (area averaging).
    
    Parameters:
    - image: NumPy 2D array (grayscale image)
    - new_width: Target width
    - new_height: Target height
    
    Returns:
    - Downsampled image as a NumPy 2D array
    """
    old_height, old_width = image.shape

    # Scale factors
    scale_x = old_width / new_width
    scale_y = old_height / new_height

    # Create output image
    downsampled = np.zeros((new_height, new_width), dtype=np.uint8)

    for y in range(new_height):
        for x in range(new_width):
            # Define the corresponding region in the original image
            start_x = int(x * scale_x)
            end_x = min(int((x + 1) * scale_x), old_width)
            start_y = int(y * scale_y)
            end_y = min(int((y + 1) * scale_y), old_height)

            # Compute the mean intensity of the region
            region = image[start_y:end_y, start_x:end_x]
            downsampled[y, x] = np.mean(region).astype(np.uint8)

    return downsampled

# Apply manual INTER_AREA downsampling
downsampled_image_manual = inter_area_downsample(thresholded_image, 28, 28)
cv2.imwrite("../data/test_8_downsampled_manual_inter_area_28x28.png", downsampled_image_manual)

# Step 2 (Alternate): Downsample using OpenCV's Bilinear for comparison
downsampled_image_bilinear = cv2.resize(thresholded_image, (28, 28), interpolation=cv2.INTER_LINEAR)
cv2.imwrite("../data/test_8_downsampled_bilinear_28x28.png", downsampled_image_bilinear)

# Step 3: Apply a Second Threshold for Contrast Boost
threshold_value = 200
_, thresholded_image = cv2.threshold(downsampled_image_area, threshold_value, 255, cv2.THRESH_BINARY)
cv2.imwrite("../data/test_8_downsampled_28x28_contrast_boosted.png", thresholded_image)

# ================= APPLYING DSP TECHNIQUES =================== #

def apply_gaussian_blur(img, ksize=3):
    return cv2.GaussianBlur(img, (ksize, ksize), 0)

def apply_bilateral_filter(img, d=9, sigmaColor=75, sigmaSpace=75):
    return cv2.bilateralFilter(img, d, sigmaColor, sigmaSpace)
def resize_bicubic(img, size=(28, 28)):
    return cv2.resize(img, size, interpolation=cv2.INTER_CUBIC)

def apply_morphological_smoothing(img, kernel_size=3):
    _, binary = cv2.threshold(img, 128, 255, cv2.THRESH_BINARY)
    kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (kernel_size, kernel_size))
    opened = cv2.morphologyEx(binary, cv2.MORPH_OPEN, kernel)
    closed = cv2.morphologyEx(opened, cv2.MORPH_CLOSE, kernel)
    return closed

def apply_fourier_smoothing(img, cutoff=5):
    f = np.fft.fft2(img)
    fshift = np.fft.fftshift(f)
    rows, cols = img.shape
    crow, ccol = rows // 2, cols // 2
    mask = np.zeros((rows, cols), np.uint8)
    mask[crow-cutoff:crow+cutoff+1, ccol-cutoff:ccol+cutoff+1] = 1
    fshift_filtered = fshift * mask
    img_back = np.fft.ifft2(np.fft.ifftshift(fshift_filtered))
    img_back = np.abs(img_back)
    img_back = (img_back / img_back.max() * 255).astype(np.uint8)
    return img_back

gaussian_blurred = apply_gaussian_blur(thresholded_image)
cv2.imwrite("../data/test_8_gaussian_blur.png", gaussian_blurred)

normalized = cv2.normalize(gaussian_blurred, None, alpha=-20, beta=255, norm_type=cv2.NORM_MINMAX)
cv2.imwrite("../data/test_8_gaussian_blur_darkened.png", normalized)

inverted = 255-normalized
cv2.imwrite("../data/test_8_gaussian_blur_darkened_inverted.png", inverted)

bilateral_filtered = apply_bilateral_filter(thresholded_image)
cv2.imwrite("../data/test_8_bilateral_filter.png", bilateral_filtered)

bicubic_resized = resize_bicubic(thresholded_image)
cv2.imwrite("../data/test_8_bicubic_resized.png", bicubic_resized)

morph_smoothed = apply_morphological_smoothing(thresholded_image)
cv2.imwrite("../data/test_8_morphological_smoothing.png", morph_smoothed)

fourier_smoothed = apply_fourier_smoothing(thresholded_image)
cv2.imwrite("../data/test_8_fourier_smoothing.png", fourier_smoothed)
