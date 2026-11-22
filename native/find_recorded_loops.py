import sys,pathlib,json,numpy as np
sys.path.insert(0,'build/audio-tools')
import soundfile as sf
out=pathlib.Path('build/recorded-bgm');result=[]
for p in out.glob('*.wav'):
 x,r=sf.read(p,dtype='float32');mono=x.mean(axis=1);hop=r//10;n=len(mono)//hop
 spec=np.abs(np.fft.rfft(mono[:n*hop].reshape(n,hop)*np.hanning(hop),axis=1))
 edges=np.unique(np.geomspace(2,1600,100).astype(int))
 f=np.stack([np.sqrt((spec[:,a:b]**2).sum(axis=1)) for a,b in zip(edges[:-1],edges[1:])],axis=1);f/=np.maximum(np.linalg.norm(f,axis=1,keepdims=True),1e-9)
 candidates=[]
 for lag in range(max(150,n//2-350),n//2+50):
  end=n-lag-150
  if end<=350:continue
  candidates.append((float(np.mean((f[300:end]-f[300+lag:end+lag])**2)),lag/10))
 _,period=min(candidates)
 # Refine against waveform, with the whole 20s window after the introduction.
 a=mono[30*r:50*r:4].astype('float64');lo=int((30+period-.2)*r)//4;hi=int((50+period+.2)*r)//4
 b=mono[::4][lo:hi].astype('float64');size=1<<(len(a)+len(b)-1).bit_length()
 c=np.fft.irfft(np.fft.rfft(b,size)*np.fft.rfft(a[::-1],size),size)
 vals=c[len(a)-1:len(b)];k=int(np.argmax(vals));offset=(lo+k)*4-30*r
 start=30*r;end=start+offset
 corr=float(np.dot(a,b[k:k+len(a)])/np.sqrt(np.dot(a,a)*np.dot(b[k:k+len(a)],b[k:k+len(a)])))
 (out/(p.stem+'.loop')).write_text(f'{start} {end}\n')
 result.append(dict(stem=p.stem,period=offset/r,correlation=corr,start=start,end=end));print(result[-1],flush=True)
(out/'loops.json').write_text(json.dumps(result,indent=2))
