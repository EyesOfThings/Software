package eu.eyesofthings.googleplayskeletonapp.UI.Page;

import android.app.Fragment;
import android.content.Context;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import eu.eyesofthings.googleplayskeletonapp.R;
import eu.eyesofthings.googleplayskeletonapp.UI.GUI;

public class CreditsPage extends Fragment implements GUI {

    private OnUpdateListener mUpdateGUICallback;
    private View mView;

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        // This makes sure that the container activity has implemented
        // the callback interface. If not, it throws an exception.
        try {
            mUpdateGUICallback = (OnUpdateListener) context;
        } catch (ClassCastException e) {
            throw new ClassCastException(context.toString()
                    + " must implement OnUpdateListener");
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        mView = inflater.inflate(R.layout.credits_page, container, false);

        TextView creditsTextView = (TextView) mView.findViewById(R.id.creditsTextView);

        String creditText = "This project has received funding from the European Union’s Horizon 2020 " +
                            "research and innovation programme under grant agreement No 643924." +
                            "\n\n\n" +
                            "Powered by:\n" +
                            "paho(http://www.eclipse.org/paho/):\n" +
                            "Copyright (c) 2007, Eclipse Foundation, Inc. and its licensors.\n" +
                            "\n" +
                            "All rights reserved.\n" +
                            "\n" +
                            "Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:\n" +
                            "\n" +
                            "* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.\n" +
                            "* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.\n" +
                            "* Neither the name of the Eclipse Foundation, Inc. nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.\n" +
                            "\n" +
                            "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, " +
                            "INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. " +
                            "IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR " +
                            "CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; " +
                            "OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING " +
                            "NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.";
        creditsTextView.setText( creditText );

        return mView;
    }

    @Override
    public void onStart() {
        super.onStart();
        // During startup, check if there are arguments passed to the fragment.
        // onStart is a good place to do this because the layout has already been
        // applied to the fragment at this point so we can safely call the method
        // below that sets the article text.
        mUpdateGUICallback.onUpdate();
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
    }

    @Override
    public void update() {
    }

}
