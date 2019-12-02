import numpy as np
import pandas as pd 
from pysmt.shortcuts import Symbol, And, Or, Not, Equals, is_sat, is_unsat, get_model, Int, LT, LE, GE, Plus
from pysmt.typing import INT

df = pd.read_csv("corral.csv")

# Add +1 to each variable so C0 = 0
for i in df:

	if i == "target":
		break

	df[i] += 1

# Start dictionary for variable mapping
dictionary = {}

features = df.columns.values[:-1]
allFeatLen = len(features)
target = df.columns.values[-1]

for i in features:
	dictionary[i] = {}

# Prints the dataframe
#print df, "\n"

# Get on-set and off-set
def getOnandOff(df, target):

	on, off = [],[]

	df1 = df.ix[df.target == 1]
	df2 = df.ix[df.target == 0]

	df1 = df1.drop([target], axis = 1)
	df2 = df2.drop([target], axis = 1)

	return df1, df2

# On = 1
# Off = 0
on,off = getOnandOff(df, target)

rules = 1

x = df.drop(target, axis = 1)
y = df.drop(features, axis = 1)

def encode(rules):

	domain = []
	constraints = {}

	for i in features:

		# Create var
		var = Symbol(i,INT)

		# Get unique names in each feature
		uniqueLen = len(df[i].unique())

		ind = 0

		dictionary[i].update({"c0":ind})

		for j in df[i].unique():
			dictionary[i].update({j:(ind + 1)})
			ind += 1

	# Constraint 28

	index = 1

	for i in range(1, rules + 1):
		for j in features:

			sym = Symbol(str(index), INT)

			name = "l_" + str(i) + j
			constraints[name] = index
			index += 1

			ub = len(dictionary[j]) - 1
			domain.append(And(LE(sym, Int(ub)), GE(sym, Int(0))))
			domain.append(Not(Equals(sym, Int(0))))


	# Constraint 29

	for i in range(1, rules + 1):
		for j in features:
			for k in range(1, len(dictionary[j])):

				sym = Symbol(str(index), INT)

				name = "d" + str(k) + "_" + str(i) + str(j)
				constraints[name] = index
				index += 1

				nameLJR = "l_" + str(i) + str(j)
				ljr = constraints[nameLJR]

				getSym = Symbol(str(ljr), INT)

				domain.append(And(Not(Equals(getSym, Int(k))), Not(Equals(getSym, Int(0)))))

	# Constraint 30

	#for i in range(0, len(x)):
	#	print x.iloc[i]['Category']

	andCRClauses = []
	allClauses = []

	for i in range(1, rules + 1):
		for j in range(0, len(x)):

			#print orCRClauses

			if len(andCRClauses) != 0:
				allClauses.append(andCRClauses)

			andCRClauses = []

			for k in features:

				sym = Symbol(str(index), INT)

				name = "c" + str(k) + "_" + str(i) + str(j)
				constraints[name] = index
				index += 1

				featValue = x.iloc[j][k]
				nameDjr = "d" + str(featValue) + "_" + str(i) + str(k)
				djr = constraints[nameDjr]
				getSym = Symbol(str(djr), INT)

				andCRClauses.append(getSym)

	andReunion = []
	
	for i in allClauses:

		for j in i:
			andReunion.append(Not(Equals(j, Int(0))))

	chunks = [andReunion[wut:wut+allFeatLen] for wut in xrange(0, len(andReunion), allFeatLen)]

	for i in chunks:
		domain.append(And(i))

	# Constraint 31

	orClauses = []
	orAllClauses = []

	for i in range(1, rules + 1):
		for j in range(0, len(x)):

			if len(orClauses) != 0:
				orAllClauses.append(orClauses)

			orClauses = []

			for k in features:

				name = "c" + str(k) + "_" + str(i) + str(j)
				crjq = constraints[name]
				getSym = Symbol(str(crjq), INT)

				orClauses.append(Not(Equals(getSym, Int(0))))

	
	for i in orAllClauses:
		domain.append(Or(i))

	# Constraint 32

	

	#print dictionary,"\n"

	#print constraints,"\n"

	print domain

	return domain

clauses = []
clauses = encode(1)
#print clauses,"\n"

model = get_model(And(clauses))
print model

# while True:

# 	clauses = encode(rules)

# 	model = get_model(And(clauses))

# 	if model:
# 		print(model)
# 		break

# 	else:
# 		kTerms += 1