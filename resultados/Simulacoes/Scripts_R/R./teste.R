library(plotly)
library(plyr)
library(ggplot2)

x <- c('50 nós', '75 nos', '100 nós')
y <- c(90, 94.58, 96)
y2 <- c(91,95,96)
text <- c('27% market share', '24% market share', '19% market share')
data <- data.frame(x, y, y2, text)

p <- data %>% 
  plot_ly() %>%
  add_trace(x = x, y = y, type = 'bar', 
            text = y, textposition = 'auto',
            marker = list(color = 'rgb(158,202,225)',
                          line = list(color = 'rgb(8,48,107)', width = 1.5))) %>%
  add_trace(x = x, y = y2, type = 'bar', 
            text = y2, textposition = 'auto',
            marker = list(color = 'rgb(58,200,225)',
                          line = list(color = 'rgb(8,48,107)', width = 1.5))) %>%
  layout(title = "January 2013 Sales Report",
         barmode = 'group',
         xaxis = list(title = ""),
         yaxis = list(title = ""))

# Create a shareable link to your chart
# Set up API credentials: https://plot.ly/r/getting-started
#chart_link = api_create(p, filename="bar-grouped-labels")
#chart_link