import numpy as np
import cv2
import matplotlib.pyplot as plt
import os
import time
import tensorflow as tf
import struct
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Flatten
from tensorflow.keras.layers import Dense
from tensorflow.keras.layers import Activation
from sklearn.utils import shuffle
import sys
import pdb

def main():
	args = sys.argv[1:]
	if len(args) == 2 and args[0] == '-dataset_dir':
		dataset_dir = str(args[1])	
	else:
		dataset_dir = "./data"

	## Use CPU only
	os.environ['CUDA_VISIBLE_DEVICES'] = '-1'

	## Load MNIST dataset
	print("Loading dataset")
	train_images = []
	train_labels = []
	test_images = []
	test_labels = []

    # Resizing constants
	img_length = 20
	img_height = 15

	for i in range(4): # 0 to 4 for the categories
		read_folder = dataset_dir + "/" + str(i) + "/"
		for filename in os.listdir(read_folder):
			img = cv2.imread(os.path.join(read_folder,filename), cv2.IMREAD_COLOR)
			img = cv2.resize(img, (img_length, img_height))	# Resize img to fit dims
			if img is not None:
				train_images.append(img / 255) # normalize pixel vals to be between 0 - 1
				train_labels.append(i)

	## Convert to numpy arrays and set the correct types
	train_images = np.asarray(train_images).astype('float32')
	train_labels = np.asarray(train_labels).astype('uint8')

	## Shuffle dataset
	train_images, train_labels = shuffle(train_images, train_labels)
	nb_images = len(train_images)

	## Create test data
	test_images = train_images[0:round(nb_images*0.2)]
	train_images = train_images[round(nb_images*0.2):]
	test_labels = train_labels[0:round(nb_images*0.2)]
	train_labels = train_labels[round(nb_images*0.2):]

	## Define network structure
	model = Sequential([
		Flatten(input_shape=((img_height, img_length, 3))), # reshape 20*15*3 to 900, layer 0
		Dense(32, activation='relu', use_bias=False),	# dense layer 1
		Dense(24, activation='relu', use_bias=False),	# dense layer 2
		Dense(4, activation='softmax', use_bias=False),	# dense layer 3
	])

	model.compile(optimizer='adam',
				  loss='sparse_categorical_crossentropy',
				  metrics=['accuracy'])


	## Train network 
	print("Start training") 
	model.fit(train_images, train_labels, epochs=300, batch_size=1000, validation_split = 0.1)

	model.summary()

	start_t = time.time()
	results = model.evaluate(test_images, test_labels, verbose=0)
	totalt_t = time.time() - start_t
	print("Inference time for ", len(test_images), " test image: " , totalt_t, " seconds")

	print("test loss, test acc: ", results)

	## Retrieve network weights after training. Skip layer 0 (input layer)
	for w in range(1, len(model.layers)):
		weight_filename = "layer_" + str(w) + "_weights.txt" 
		open(weight_filename, 'w').close() # clear file
		file = open(weight_filename,"a") 
		file.write('{')
		for i in range(model.layers[w].weights[0].numpy().shape[0]):
			file.write('{')
			for j in range(model.layers[w].weights[0].numpy().shape[1]):
				file.write(str(model.layers[w].weights[0].numpy()[i][j]))
				if j != model.layers[w].weights[0].numpy().shape[1]-1:
					file.write(', ')
			file.write('}')
			if i != model.layers[w].weights[0].numpy().shape[0]-1:
				file.write(', \n')
		file.write('}')
		file.close()

	print("test_image[0] label: ", test_labels[0])

	x = test_images[0]
	x = np.expand_dims(x, axis=0)
	print("NN Prediction: ", np.argmax(model.predict(x)))

	print("Finished")	
	
if __name__=="__main__":
    main()