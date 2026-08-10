import gzip, re  
text = gzip.open('index.html.gz', 'rt', encoding='utf-8', errors='replace').read()  
