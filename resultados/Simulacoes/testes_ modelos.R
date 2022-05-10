library(ggplot2)

# ggplot(data = mtcars, aes(x = disp, y = mpg, colour = cyl, size = wt)) + 
#   geom_boxplot()

# ggplot(mtcars, aes(x = as.factor(cyl), y = mpg)) + 
#   geom_boxplot()

# ggplot(mtcars, aes(x = mpg)) + 
#   geom_histogram()

# ggplot(mtcars, aes(x = as.factor(cyl))) + 
#   geom_bar()

# ggplot(mtcars, aes(x = as.factor(cyl), y = mpg, colour = as.factor(cyl))) + 
#   geom_boxplot()

# ggplot(mtcars, aes(x = as.factor(cyl), y = mpg, fill = as.factor(cyl))) + geom_boxplot()
# 
# ggplot(mtcars, aes(x = as.factor(cyl), y = mpg)) + 
#   geom_boxplot(color = "red", fill = "pink")
# 
# ggplot(mtcars, aes(x = mpg)) + 
#   geom_histogram() +
#   xlab("Milhas por galão") +
#   ylab("Frequência")

ggplot(mtcars, aes(x = mpg)) + 
  geom_histogram() +
  xlab("Milhas por galão") +
  ylab("Frequência") +
  xlim(c(0, 40)) +
  ylim(c(0,8))