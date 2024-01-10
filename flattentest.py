import cv2
import numpy as np

# Load
image = cv2.imread('/home/branko/FPGA_AI/Bolts_Data/Testing/3/image-1587.jpg', cv2.IMREAD_GRAYSCALE)  # Read the image in greyscale

# Resize to 20x20
resized_image = cv2.resize(image, (20, 20))

#flatten
normalized_flattened_array = (resized_image.flatten() / 255.0).astype(np.float32)

# Print
print("Normalized Flattened Array:", normalized_flattened_array)
reshaped_array = normalized_flattened_array.reshape(1, -1)

# Save  to text
with open('flattened_array.txt', 'w') as file:
    for row in reshaped_array:
        row_str = ', '.join(f"{value:.6f}" for value in row)
        file.write(f"{row_str}\n")
