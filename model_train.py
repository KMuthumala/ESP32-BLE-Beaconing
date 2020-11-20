import numpy as np
import pandas as pd
from sklearn import svm
from keras.layers import Dense
from sklearn.metrics import accuracy_score
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split

dataset = pd.read_csv('iot_dataset.csv')
dataset.head()

X = dataset.iloc[:, 0:15]
y = dataset.iloc[:, 15]

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.20)

#################### Random Forest #####################
clf = RandomForestClassifier(n_estimators=400, max_depth=50)
clf.fit(X_train, y_train)
########################################################

################ Support Vector Machine ################
clf = svm.SVC(C=10, kernel='rbf', gamma=0.003)
clf.fit(X_train, y_train)
########################################################
y_train_pred = clf.predict(X_train)
y_test_pred = clf.predict(X_test)
df = pd.DataFrame({'y_test': y_test.values, 'y_test_predictions': y_test_pred})

#################### Neural Netwroks ###################
y_train = keras.utils.to_categorical(y_train, num_classes=24)
y_test = keras.utils.to_categorical(y_test, num_classes=24)

model = keras.models.Sequential()
model.add(Dense(64, input_shape=(15,), activation='relu'))
model.add(Dense(32, activation='relu'))
model.add(Dense(24, activation='softmax'))

model.compile(optimizer="Adam", loss="categorical_crossentropy", metrics=["accuracy"])
model.fit(X_train.values, y_train, batch_size=64, epochs=100)

train_loss, train_acc = model.evaluate(X_train.values, y_train, verbose=2)
test_loss, test_acc = model.evaluate(X_test.values, y_test, verbose=2)

y_test_pred = model.predict(X_test.values)
y_test_predictions = np.argmax(y_test_pred, axis=1)
df = pd.DataFrame({'y_test': np.argmax(y_test, axis=1), 'y_test_predictions': y_test_predictions})
########################################################
